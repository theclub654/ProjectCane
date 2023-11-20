#include "glob.h"

std::vector <SHD> g_ashd;
std::vector<void*> allSWAloObjs;

void LoadGlobsetFromBrx(GLOBSET* pglobset, CBinaryInputStream* pbis, ALO* palo)
{
    pglobset->cpsaa = 0;

    pbis->U8Read();

    pglobset->cbnd = pbis->U8Read();
    pglobset->mpibndoid.resize(pglobset->cbnd);

    for (int i = 0; i < pglobset->cbnd; i++)
        pglobset->mpibndoid[i] = (OID)pbis->S16Read();

    pglobset->cpose = pbis->U8Read();
    pglobset->agPoses.resize(pglobset->cpose);

    for (int i = 0; i < pglobset->cpose; i++)
        pglobset->agPoses[i] = pbis->F32Read();

    // Loading number of submodels for model
    pglobset->cglob = pbis->U16Read();

    if (pglobset->cglob != 0)
    {
        pglobset->aglob.resize(pglobset->cglob);
        pglobset->aglobi.resize(pglobset->cglob);
    }

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        uint16_t unk_5 = pbis->U16Read();
        
        if ((unk_5 & 1) != 0)
        {
            pbis->S16Read();
            pglobset->aglob[i].pdmat = pbis->ReadMatrix4x2();
        }

        if ((unk_5 & 2) != 0)
            pglobset->aglobi[i].grfzon = pbis->U32Read();

        if ((unk_5 & 0x200) != 0)
            pglobset->aglob[i].rSubglobRadius = pbis->F32Read();

        if ((unk_5 & 4) != 0)
            pbis->F32Read();

        if ((unk_5 & 8) != 0)
            pglobset->aglob[i].uFog = pbis->F32Read();

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

        pglobset->aglob[i].posCenter = pbis->ReadVector();
        pglobset->aglob[i].sRadius = pbis->F32Read();
        pbis->S16Read();
        pbis->U8Read();
        pbis->U8Read();
        pglobset->aglob[i].oid = (OID)pbis->U8Read();

        if ((unk_5 & 1) == 0)
        {
            // std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            // Number of submodels
            pglobset->aglob[i].csubglob = pbis->U16Read();

            pglobset->aglob[i].asubglob.resize(pglobset->aglob[i].csubglob);

            for (int a = 0; a < pglobset->aglob[i].csubglob; a++)
            {
                // Loading submodel origin
                pglobset->aglob[i].asubglob[a].posCenter = pbis->ReadVector();
                pglobset->aglob[i].asubglob[a].sRadius = pbis->F32Read();

                byte vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Count: " << (uint32_t)vertexCount << "\n";
                byte normalCount = pbis->U8Read();
                //std::cout << std::dec << "Rotations Count: " << (uint32_t)rotationsCount << "\n";
                byte vertexColorCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Color Count: " << (uint32_t)vertexColorCount << "\n";
                byte texcoordCount = pbis->U8Read();
                //std::cout << std::dec << "Texcoords Count: " << (uint32_t)texCoordCount << "\n";
                byte indexCount = pbis->U8Read();
                //std::cout << std::dec << "Index Count: " << (uint32_t)indexCount << "\n";
                
                pglobset->aglob[i].asubglob[a].vertexes.resize(vertexCount);
                pglobset->aglob[i].asubglob[a].normals.resize(normalCount);
                pglobset->aglob[i].asubglob[a].vertexColors.resize(vertexColorCount);
                pglobset->aglob[i].asubglob[a].texcoords.resize(texcoordCount);
                pglobset->aglob[i].asubglob[a].indexes.resize(indexCount);

                pbis->Align(4);

                //std::cout << "Vertices: " << std::hex << pbis->file.tellg() << "\n";
                
                for (int b = 0; b < vertexCount; b++)
                    pglobset->aglob[i].asubglob[a].vertexes[b] = pbis->ReadVector();
                 
                //std::cout << "Normals: " << std::hex << file.tellg() << "\n";
                for (int c = 0; c < normalCount; c++)
                    pglobset->aglob[i].asubglob[a].normals[c] = pbis->ReadVector();

                //std::cout << "Vertex Colors: " << std::hex << file.tellg() << "\n";
                for (int d = 0; d < vertexColorCount; d++)
                    pglobset->aglob[i].asubglob[a].vertexColors[d] = (RGBA)pbis->U32Read();

                //std::cout << "Texcoords: " << std::hex << pbis->file.tellg() << "\n";
                for (int e = 0; e < texcoordCount; e++)
                    pglobset->aglob[i].asubglob[a].texcoords[e] = pbis->ReadVector2();

                //std::cout << "Indexes: " << std::hex << file.tellg() << "\n\n";
                for (int f = 0; f < indexCount; f++)
                    pglobset->aglob[i].asubglob[a].indexes[f] = (VTXFLG)pbis->U32Read();

                pglobset->aglob[i].asubglob[a].indices = ConvertStripsToTriLists(pglobset->aglob[i].asubglob[a].indexes);

                pglobset->aglob[i].asubglob[a].pshd = &g_ashd[pbis->U16Read()];
                
                pglobset->aglob[i].asubglob[a].unSelfIllum = pbis->U8Read();
                pglobset->aglob[i].asubglob[a].cibnd = pbis->U8Read();

                pbis->file.seekg(pglobset->aglob[i].asubglob[a].cibnd, SEEK_CUR);
                pbis->file.seekg(vertexCount * pglobset->aglob[i].asubglob[a].cibnd * 4, SEEK_CUR);

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
            }

            if (pglobset->aglob[i].csubglob != 0)
            {
                allSWAloObjs.push_back(palo);
                pglobset->aglob[i].asubglob = MakeGLBuffers(pglobset->aglob[i].asubglob);
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

std::vector <byte> ConvertStripsToTriLists(std::vector <VTXFLG> indexes)
{
    std::vector <byte> indices;

    uint32_t idx = 0;

    for (int i = 2; i < indexes.size(); i++)
    {
        byte indice0 = indexes[idx + 0].ipos;
        byte indice1 = indexes[idx + 1].ipos;
        byte indice2 = indexes[idx + 2].ipos;
        byte stripFlag = indexes[idx + 2].bMisc;

        if (stripFlag != 0x80 && stripFlag != 0x81 && stripFlag != 0x82 && stripFlag != 0x83 && stripFlag != 0x84 && stripFlag != 0x85 && stripFlag != 0x86 && stripFlag != 0x87 && stripFlag != 0x88 && stripFlag != 0x89 && stripFlag != 0x8A && stripFlag != 0x8B && stripFlag != 0x8C && stripFlag != 0x8D && stripFlag != 0x8E && stripFlag != 0x8F && stripFlag != 0xFF)
        {
            indices.push_back(indice0);
            indices.push_back(indice1);
            indices.push_back(indice2);
        }
        idx++;
    }

    return indices;
}

std::vector <SUBGLOB> MakeGLBuffers(std::vector<SUBGLOB> asubglob)
{
    for (int i = 0; i < asubglob.size(); i++)
    {
        glGenVertexArrays(1, &asubglob[i].VAO);
        glBindVertexArray(asubglob[i].VAO);

        glGenBuffers(1, &asubglob[i].VBO);
        glBindBuffer(GL_ARRAY_BUFFER, asubglob[i].VBO);
        glBufferData(GL_ARRAY_BUFFER, asubglob[i].vertexes.size() * sizeof(glm::vec3), asubglob[i].vertexes.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &asubglob[i].VNO);
        glBindBuffer(GL_ARRAY_BUFFER, asubglob[i].VNO);
        glBufferData(GL_ARRAY_BUFFER, asubglob[i].normals.size() * sizeof(glm::vec3), asubglob[i].normals.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &asubglob[i].VCB);
        glBindBuffer(GL_ARRAY_BUFFER, asubglob[i].VCB);
        glBufferData(GL_ARRAY_BUFFER, asubglob[i].vertexColors.size() * sizeof(uint32_t), asubglob[i].vertexColors.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_TRUE, sizeof(uint32_t), (void*)0);
        glEnableVertexAttribArray(2);

        glGenBuffers(1, &asubglob[i].TCB);
        glBindBuffer(GL_ARRAY_BUFFER, asubglob[i].TCB);
        glBufferData(GL_ARRAY_BUFFER, asubglob[i].texcoords.size() * sizeof(glm::vec2), asubglob[i].texcoords.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(3);

        glGenBuffers(1, &asubglob[i].EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, asubglob[i].EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, asubglob[i].indices.size() * sizeof(uint8_t), asubglob[i].indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    return asubglob;
}