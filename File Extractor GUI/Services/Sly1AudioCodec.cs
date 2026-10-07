using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO;

namespace File_Extractor_GUI.Services;

internal static class Sly1AudioCodec
{
    public static byte[] DecodeVag(byte[] vag)
    {
        if (vag.Length < 0x30 || vag[0] != 'V' || vag[1] != 'A' || vag[2] != 'G' || vag[3] != 'p')
            throw new InvalidDataException("Missing Sony VAGp header.");

        var declared = ReadBe32(vag, 0x0C);
        var rate = ReadBe32(vag, 0x10);
        if (rate is < 4000 or > 192000)
            throw new InvalidDataException($"Invalid VAG sample rate {rate}.");

        var available = vag.Length - 0x30;
        var size = declared == 0 ? available : (int)Math.Min(declared, (uint)available);
        size &= ~15;
        return WriteWav(DecodeAdpcm(vag.AsSpan(0x30, size)), null, 1, rate);
    }

    public static (byte[] Video, byte[] Audio) DemuxPss(byte[] pss)
    {
        const uint End = 0x000001B9, Pack = 0x000001BA, System = 0x000001BB;
        const uint Private = 0x000001BD, Padding = 0x000001BE, Video = 0x000001E0;
        var video = new List<byte>();
        var sonyAds = new List<byte>();
        var directAudio = new List<byte>();
        uint format = 0, rate = 0, interleave = 0x800;
        byte channels = 0;
        var firstAudio = true;
        var offset = 0;

        while (offset + 4 <= pss.Length)
        {
            var id = ReadBe32(pss, offset);
            if (id == End) break;
            if (id == Pack)
            {
                if (offset + 14 > pss.Length) throw new InvalidDataException("Truncated PSS pack header.");
                offset += 14 + (pss[offset + 13] & 7);
                continue;
            }
            if (id is System or Padding)
            {
                if (offset + 6 > pss.Length) throw new InvalidDataException("Truncated PSS packet.");
                offset += 6 + ReadBe16(pss, offset + 4);
                continue;
            }
            if (id != Video && id != Private) { offset++; continue; }
            if (offset + 9 > pss.Length) throw new InvalidDataException("Truncated PSS PES packet.");
            var packetSize = ReadBe16(pss, offset + 4);
            if (offset + 6 + packetSize > pss.Length) throw new InvalidDataException("Invalid PSS packet size.");
            var extension = pss[offset + 8];
            var payload = offset + 9 + extension;
            var payloadSize = packetSize - 3 - extension;
            if (payloadSize < 0 || payload + payloadSize > pss.Length) throw new InvalidDataException("Invalid PSS payload.");

            if (id == Video)
                video.AddRange(pss.AsSpan(payload, payloadSize).ToArray());
            else if (payloadSize > 4)
            {
                sonyAds.AddRange(pss.AsSpan(payload + 4, payloadSize - 4).ToArray());
                if (pss[payload + 3] == 1)
                {
                    var audioOffset = payload + 4;
                    var audioSize = payloadSize - 4;
                    if (firstAudio)
                    {
                        if (audioSize < 0x800) throw new InvalidDataException("Truncated first PSS audio packet.");
                        rate = ReadBe16(pss, audioOffset + 6);
                        channels = pss[audioOffset + 8];
                        format = 2;
                        audioOffset += 0x800;
                        audioSize -= 0x800;
                        firstAudio = false;
                    }
                    directAudio.AddRange(pss.AsSpan(audioOffset, audioSize).ToArray());
                }
            }
            offset += 6 + packetSize;
        }

        var adpcm = directAudio.ToArray();
        if (adpcm.Length == 0)
        {
            var ads = sonyAds.ToArray();
            var header = Find(ads, "SShd"u8);
            var body = header < 0 ? -1 : Find(ads, "SSbd"u8, header + 8);
            if (header < 0 || body < 0 || header + 32 > ads.Length || body + 8 > ads.Length)
                throw new InvalidDataException("PSS audio headers were not found.");
            format = ReadLe32(ads, header + 8);
            rate = ReadLe32(ads, header + 12);
            channels = checked((byte)ReadLe32(ads, header + 16));
            interleave = ReadLe32(ads, header + 20);
            var size = (int)Math.Min(ReadLe32(ads, body + 4), (uint)(ads.Length - body - 8));
            adpcm = ads.AsSpan(body + 8, size).ToArray();
        }

        if (video.Count == 0 || adpcm.Length == 0 || channels is < 1 or > 2 || rate == 0)
            throw new InvalidDataException("The PSS streams are incomplete.");

        var channelBytes = new[] { new List<byte>(), new List<byte>() };
        if (channels == 1) channelBytes[0].AddRange(adpcm);
        else
        {
            var position = 0; var channel = 0; var block = (int)(interleave == 0 ? 0x800 : interleave);
            while (position < adpcm.Length)
            {
                var count = Math.Min(block, adpcm.Length - position);
                channelBytes[channel].AddRange(adpcm.AsSpan(position, count).ToArray());
                position += count; channel ^= 1;
            }
        }

        short[] left, right = [];
        if (format is 2 or 0x10)
        {
            left = DecodeAdpcm(channelBytes[0].ToArray());
            if (channels == 2) right = DecodeAdpcm(channelBytes[1].ToArray());
        }
        else if (format is 0 or 1)
        {
            left = DecodePcm(channelBytes[0].ToArray(), format == 1);
            if (channels == 2) right = DecodePcm(channelBytes[1].ToArray(), format == 1);
        }
        else
        {
            throw new InvalidDataException($"Unsupported PSS audio format {format}.");
        }
        return (video.ToArray(), WriteWav(left, right, channels, rate));
    }

    private static short[] DecodeAdpcm(ReadOnlySpan<byte> data)
    {
        int[,] filters = { { 0, 0 }, { 60, 0 }, { 115, -52 }, { 98, -55 }, { 122, -60 } };
        var pcm = new List<short>(data.Length * 2); var h1 = 0; var h2 = 0;
        for (var offset = 0; offset + 16 <= data.Length; offset += 16)
        {
            var frame = data.Slice(offset, 16); var predictor = frame[0] >> 4; var shift = frame[0] & 15;
            if (predictor >= 5) continue;
            var end = frame[0] == 0 && frame[1] == 1;
            for (var i = 2; end && i < 16; i++) end = frame[i] == 0x77;
            if (end) break;
            for (var i = 0; i < 28; i++)
            {
                var packed = frame[2 + i / 2]; var nibble = (i & 1) != 0 ? packed >> 4 : packed & 15;
                if (nibble >= 8) nibble -= 16;
                var sample = (nibble << 12) >> shift;
                sample += (h1 * filters[predictor, 0] + h2 * filters[predictor, 1] + 32) >> 6;
                sample = Math.Clamp(sample, short.MinValue, short.MaxValue); h2 = h1; h1 = sample; pcm.Add((short)sample);
            }
        }
        return pcm.ToArray();
    }

    private static short[] DecodePcm(byte[] data, bool littleEndian)
    {
        var samples = new short[data.Length / 2];
        for (var i = 0; i < samples.Length; i++)
            samples[i] = littleEndian ? BinaryPrimitives.ReadInt16LittleEndian(data.AsSpan(i * 2, 2)) : BinaryPrimitives.ReadInt16BigEndian(data.AsSpan(i * 2, 2));
        return samples;
    }

    private static byte[] WriteWav(short[] left, short[]? right, int channels, uint rate)
    {
        var frames = channels == 2 ? Math.Min(left.Length, right?.Length ?? 0) : left.Length;
        if (frames == 0) throw new InvalidDataException("Audio decoder produced no samples.");
        var dataSize = checked(frames * channels * 2); using var output = new MemoryStream(44 + dataSize); using var writer = new BinaryWriter(output);
        writer.Write("RIFF"u8); writer.Write(36 + dataSize); writer.Write("WAVEfmt "u8); writer.Write(16); writer.Write((ushort)1); writer.Write((ushort)channels);
        writer.Write(rate); writer.Write(rate * (uint)channels * 2); writer.Write((ushort)(channels * 2)); writer.Write((ushort)16); writer.Write("data"u8); writer.Write(dataSize);
        for (var i = 0; i < frames; i++) { writer.Write(left[i]); if (channels == 2) writer.Write(right![i]); }
        return output.ToArray();
    }

    private static int Find(byte[] data, ReadOnlySpan<byte> pattern, int startIndex = 0)
    {
        for (var i = Math.Max(0, startIndex); i <= data.Length - pattern.Length; i++) if (data.AsSpan(i, pattern.Length).SequenceEqual(pattern)) return i;
        return -1;
    }
    private static ushort ReadBe16(byte[] d, int o) => BinaryPrimitives.ReadUInt16BigEndian(d.AsSpan(o, 2));
    private static uint ReadBe32(byte[] d, int o) => BinaryPrimitives.ReadUInt32BigEndian(d.AsSpan(o, 4));
    private static uint ReadLe32(byte[] d, int o) => BinaryPrimitives.ReadUInt32LittleEndian(d.AsSpan(o, 4));
}
