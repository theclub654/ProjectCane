#!/usr/bin/env python3
"""Synthetic rejection tests; optionally validate a local disc without extracting it."""
import argparse, pathlib, subprocess, tempfile
p=argparse.ArgumentParser();p.add_argument('extractor',type=pathlib.Path);p.add_argument('--iso',type=pathlib.Path);a=p.parse_args()
exe=str(a.extractor.resolve())
with tempfile.TemporaryDirectory(prefix='cane extractor ') as folder:
 root=pathlib.Path(folder);out=root/'output';invalid=root/'invalid.iso';invalid.write_bytes(b'not an ISO')
 for image in [root/'missing.iso',invalid]:
  result=subprocess.run([exe,'--iso',str(image),'--output',str(out),'--non-interactive','--progress=jsonl'],capture_output=True,timeout=10)
  assert result.returncode!=0 and not out.exists(),result
 result=subprocess.run([exe,'--iso','--non-interactive'],capture_output=True,timeout=10)
 assert result.returncode!=0
 if a.iso:
  result=subprocess.run([exe,'--iso',str(a.iso),'--validate-only','--progress=jsonl'],capture_output=True,timeout=10)
  assert result.returncode==0,result.stderr
  # A plausible volume size with a different executable must be rejected.
  with a.iso.open('rb') as source:head=bytearray(source.read(0x8a000+1712328))
  head[0x8a000+100]^=1
  unsupported=root/'unsupported.iso'
  with unsupported.open('wb') as image:image.write(head);image.truncate(a.iso.stat().st_size)
  result=subprocess.run([exe,'--iso',str(unsupported),'--validate-only','--non-interactive'],capture_output=True,timeout=10)
  assert result.returncode!=0 and b'Unsupported' in result.stderr,'Different executable revision accepted'
  # Keep the recognized executable but truncate all game assets.
  with a.iso.open('rb') as source:head=source.read(0x8a000+1712328)
  truncated=root/'truncated.iso';truncated.write_bytes(head)
  result=subprocess.run([exe,'--iso',str(truncated),'--validate-only','--non-interactive'],capture_output=True,timeout=10)
  assert result.returncode!=0,'Truncated disc accepted'
print('Extractor rejection tests passed')
