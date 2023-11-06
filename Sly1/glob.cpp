#include "glob.h"
#include "shd.h"

void LoadGlobsetFromBrx(GLOBSET *pglobset, CBinaryInputStream *pbis)
{
    pglobset->cpsaa = 0;

	pbis->U8Read();
	pglobset->cbnd = pbis->U8Read();
    
    pglobset->mpibndoid = (OID*) new OID[pglobset->cbnd << 2]();

    for (int i = 0; i < pglobset->cbnd; i++)
        pglobset->mpibndoid[i] = (OID)pbis->S16Read();
    
    pglobset->cpose = pbis->U8Read();
    pbis->file.seekg(pglobset->cpose << 2, SEEK_CUR);

    // Loading number of submodels for model
    pglobset->cglob = pbis->U16Read();
    
    // Allocating memory for submodel data for model
    GLOB *glob = (GLOB*)new GLOB[pglobset->cglob << 7]();
    pglobset->aglob = glob;
    GLOBI* globi = (GLOBI*)new GLOBI[pglobset->cglob * 0x28]();
    pglobset->aglobi = globi;

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        pglobset->aglob = pglobset->aglob;
        pglobset->aglobi = pglobset->aglobi;
        
        uint16_t unk_5 = pbis->U16Read();
        
        if ((unk_5 & 1) != 0)
        {
            pbis->S16Read();
            pglobset->aglob->pdmat = pbis->ReadMatrix4x2();
        }

        if ((unk_5 & 2) != 0)
            pglobset->aglobi->grfzon = pbis->U32Read();

        if ((unk_5 & 0x200) != 0)
            pglobset->aglob->rSubglobRadius = pbis->F32Read();

        if ((unk_5 & 4) != 0)
            pbis->F32Read();

        if ((unk_5 & 8) != 0)
            pglobset->aglob->uFog = pbis->F32Read();

        if ((unk_5 & 0x10) != 0)
            pbis->F32Read();

        if ((unk_5 & 0x20) != 0)
            pbis->F32Read();

        if ((unk_5 & 0x40) != 0)
            PsaaLoadFromBrx(pbis);

        if ((unk_5 & 0x80) != 0)
        {
            pbis->ReadVector();
            pbis->ReadVector4();
        }

        if ((unk_5 & 0x100) != 0)
        {

            pbis->S16Read();
            int8_t unk_8 = pbis->S8Read();

            if (unk_8 != -1)
            {
                pbis->F32Read();
                pbis->F32Read();
                pbis->F32Read();
                pbis->F32Read();
                pbis->ReadMatrix4x2();
            }

            pbis->U8Read();
            pbis->U8Read();
            pbis->U8Read();
        }

        pglobset->aglob->posCenter = pbis->ReadVector();
        pglobset->aglob->sRadius = pbis->F32Read();
        pbis->S16Read();
        pbis->U8Read();
        pbis->U8Read();
        pglobset->aglob->oid = (OID)pbis->U8Read();

        if ((unk_5 & 1) == 0)
        {
            // std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            // Number of submodels
            pglobset->aglob->csubglob = pbis->U16Read();

            pglobset->aglob->asubglob = (SUBGLOB*)new SUBGLOB[pglobset->aglob->csubglob * 0x50]();

            for (int i = 0; i < pglobset->aglob->csubglob; i++)
            {
                // Loading submodel origin
                pglobset->aglob->asubglob->posCenter = pbis->ReadVector();
                pglobset->aglob->asubglob->sRadius = pbis->F32Read();

                byte vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Count: " << (uint32_t)vertexCount << "\n";
                byte normalCount = pbis->U8Read();
                //std::cout << std::dec << "Rotations Count: " << (uint32_t)rotationsCount << "\n";
                byte vertexColorCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Color Count: " << (uint32_t)vertexColorCount << "\n";
                byte texCoordCount = pbis->U8Read();
                //std::cout << std::dec << "Texcoords Count: " << (uint32_t)texCoordCount << "\n";
                byte indexCount = pbis->U8Read();
                //std::cout << std::dec << "Index Count: " << (uint32_t)indexCount << "\n";

                pbis->Align(4);

                //std::cout << "Vertices: " << std::hex << pbis->file.tellg() << "\n";
                glm::vec3 *pos = (glm::vec3*)new glm::vec3[vertexCount * 0xC]();
                pbis->file.read((char*)pos, vertexCount * 0xC);

                //std::cout << "Normals: " << std::hex << file.tellg() << "\n";
                glm::vec3 *normal = (glm::vec3*)new glm::vec3[normalCount * 0xC]();
                pbis->file.read((char*)normal, normalCount * 0xC);

                //std::cout << "Vertex Colors: " << std::hex << file.tellg() << "\n";
                for (int i = 0; i < vertexColorCount; i++)
                    pbis->U32Read();

                //std::cout << "Texcoords: " << std::hex << file.tellg() << "\n";
                glm::vec2 *uv = (glm::vec2*)new glm::vec2[texCoordCount << 3]();
                pbis->file.read((char*)uv, texCoordCount << 3);

                //std::cout << "Indexes: " << std::hex << file.tellg() << "\n\n";

                for (int i = 0; i < indexCount; i++)
                {
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                    pbis->U8Read();
                }

                uint16_t shaderID = pbis->U16Read();

                pglobset->aglob->asubglob->unSelfIllum = pbis->U8Read();
                pglobset->aglob->asubglob->cibnd = pbis->U8Read();

                for (int i = 0; i < pglobset->aglob->asubglob->cibnd; i++)
                    pbis->U8Read();

                pbis->file.seekg(vertexCount * pglobset->aglob->asubglob->cibnd * 4, SEEK_CUR);

                if (pglobset->cpose != 0)
                {
                    uint16_t vertexCount1 = pbis->U16Read();

                    for (int i = 0; i < vertexCount1; i++)
                        pbis->ReadVector();

                    uint16_t normalCount = pbis->U16Read();

                    for (int i = 0; i < normalCount; i++)
                        pbis->ReadVector();

                    for (int i = 0; i < pglobset->cpose; i++)
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
                //pglob->asubglob++;
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

                if (pglobset->cpose != 0)
                {
                    uint16_t vertexCount3 = pbis->U16Read();

                    for (int i = 0; i < vertexCount3; i++)
                        pbis->ReadVector();

                    for (int i = 0; i < pglobset->cpose; i++)
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
}