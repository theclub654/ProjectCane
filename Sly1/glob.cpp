#include "glob.h"
#include "shd.h"

byte LoadGlobsetFromBrx(CBinaryInputStream *pbis)
{
	byte unk0 = pbis->U8Read();
	byte unk1 = pbis->U8Read();

    for (int i = 0; i < unk1; i++)
        pbis->S16Read();

    byte unk_3 = pbis->U8Read();
    pbis->file.seekg(unk_3 << 2, SEEK_CUR);
    uint16_t unk_4 = pbis->U16Read();

    for (int i = 0; i < unk_4; i++)
    {
        uint16_t unk_5 = pbis->U16Read();

        if ((unk_5 & 1) != 0)
        {
            pbis->S16Read();

            pbis->F32Read(); pbis->F32Read(); pbis->F32Read();
            pbis->F32Read(); pbis->F32Read(); pbis->F32Read();
            pbis->F32Read(); pbis->F32Read(); pbis->F32Read();
            pbis->F32Read(); pbis->F32Read(); pbis->F32Read();
        }

        if ((unk_5 & 2) != 0)
            pbis->U32Read();

        if ((unk_5 & 0x200) != 0)
            pbis->F32Read();;

        if ((unk_5 & 4) != 0)
            pbis->F32Read();;

        if ((unk_5 & 8) != 0)
            pbis->F32Read();;

        if ((unk_5 & 0x10) != 0)
            pbis->F32Read();;

        if ((unk_5 & 0x20) != 0)
            pbis->F32Read();;

        if ((unk_5 & 0x40) != 0)
            PsaaLoadFromBrx(pbis);

        if ((unk_5 & 0x80) != 0)
        {
            pbis->ReadVector();
            pbis->ReadVector4();
        }

        if ((unk_5 & 0x100) != 0)
        {

            int16_t unk_7 = pbis->S16Read();
            int8_t unk_8 = pbis->S8Read();

            if (unk_8 != -1)
            {
                pbis->F32Read();;
                pbis->F32Read();;
                pbis->F32Read();;
                pbis->F32Read();;
                pbis->file.seekg(0x30, SEEK_CUR); //ReadMatrix4
            }

            pbis->U8Read();
            pbis->U8Read();
            pbis->U8Read();
        }

        pbis->ReadVector();
        pbis->F32Read();;
        int16_t unk_9 = pbis->S16Read();
        byte unk_10 = pbis->U8Read();
        byte unk_11 = pbis->U8Read();
        byte unk_12 = pbis->U8Read();

        if ((unk_5 & 1) == 0)
        {
            //std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            uint16_t numSubMesh0 = pbis->U16Read();

            for (int i = 0; i < numSubMesh0; i++)
            {
                pbis->ReadVector();
                pbis->F32Read();;

                byte vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Count: " << (uint32_t)vertexCount << "\n";
                byte normalsCount = pbis->U8Read();
                //std::cout << std::dec << "Rotations Count: " << (uint32_t)rotationsCount << "\n";
                byte vertexColorCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Color Count: " << (uint32_t)vertexColorCount << "\n";
                byte texCoordCount = pbis->U8Read();
                //std::cout << std::dec << "Texcoords Count: " << (uint32_t)texCoordCount << "\n";
                byte indexCount = pbis->U8Read();
                //std::cout << std::dec << "Index Count: " << (uint32_t)indexCount << "\n";

                pbis->Align(4);

                //std::cout << "Vertices: " << std::hex << file.tellg() << "\n";
                for (int i = 0; i < vertexCount; i++)
                    pbis->ReadVector();

                //std::cout << "Normals: " << std::hex << file.tellg() << "\n";
                for (int i = 0; i < normalsCount; i++)
                    pbis->ReadVector();

                //std::cout << "Vertex Colors: " << std::hex << file.tellg() << "\n";
                for (int i = 0; i < vertexColorCount; i++)
                    pbis->U32Read();

                //std::cout << "Texcoords: " << std::hex << file.tellg() << "\n";
                for (int i = 0; i < texCoordCount; i++)
                {
                    pbis->F32Read();
                    pbis->F32Read();
                }

                //std::cout << "Indexes: " << std::hex << file.tellg() << "\n\n";

                for (int i = 0; i < indexCount; i++)
                {
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                }

                uint16_t textureID = pbis->U16Read();

                byte unk_19 = pbis->U8Read();
                byte unk_20 = pbis->U8Read();

                for (int i = 0; i < unk_20; i++)
                    pbis->U8Read();

                pbis->file.seekg(vertexCount * unk_20 * 4, SEEK_CUR);

                if (unk_3 != 0)
                {
                    uint16_t vertexCount1 = pbis->U16Read();

                    for (int i = 0; i < vertexCount1; i++)
                        pbis->ReadVector();

                    uint16_t normalCount = pbis->U16Read();

                    for (int i = 0; i < normalCount; i++)
                        pbis->ReadVector();

                    for (int i = 0; i < unk_3; i++)
                    {
                        for (int i = 0; i < indexCount; i++)
                        {
                            pbis->U8Read();
                            pbis->U8Read();
                        }

                        for (int i = 0; i < indexCount; i++)
                        {
                            pbis->U8Read();
                            pbis->U8Read();
                        }
                    }
                }
            }

            uint16_t numSubMesh1 = pbis->U16Read();

            for (int i = 0; i < numSubMesh1; i++)
            {

                byte vertexCount2 = pbis->U8Read();

                for (int i = 0; i < vertexCount2; i++)
                    pbis->ReadVector();

                byte indexCount = pbis->U8Read();

                for (int i = 0; i < indexCount; i++)
                {
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                }

                byte unk_26 = pbis->U8Read();

                for (int i = 0; i < unk_26; i++)
                    pbis->U8Read();

                pbis->file.seekg(unk_26 * vertexCount2 * 4, SEEK_CUR);

                if (unk_3 != 0)
                {
                    uint16_t vertexCount3 = pbis->U16Read();

                    for (int i = 0; i < vertexCount3; i++)
                        pbis->ReadVector();

                    for (int i = 0; i < unk_3; i++)
                    {
                        for (int i = 0; i < vertexCount2; i++)
                        {
                            pbis->U16Read();
                        }
                    }
                }
            }
        }
    }

    return unk_3;
}
