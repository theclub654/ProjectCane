#include "glob.h"

std::vector <SHD> g_ashd;
extern std::vector<ALO*> allSWAloObjs;
extern std::vector<LIGHT*> allSwLights;

void LoadGlobsetFromBrx(GLOBSET* pglobset ,CBinaryInputStream* pbis, ALO* palo)
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

    pglobset->aglob.resize(pglobset->cglob);
    pglobset->aglobi.resize(pglobset->cglob);

    // Loading each submodel for a model
    for (int i = 0; i < pglobset->cglob; i++)
    {
        uint16_t unk_5 = pbis->U16Read();
        
        if ((unk_5 & 1) != 0)
        {
            int instanceIndex = pbis->S16Read();
            glm::mat4 pdmat = pbis->ReadMatrix4();

            if (instanceIndex != 0)
                pglobset->aglob[instanceIndex].pdmat.push_back(pdmat);
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
            WRBG wrbg{};
            WRBG* pwrbg = &wrbg;

            pwrbg->oid = (OID)pbis->S16Read();
            pwrbg->weki.wek = (WEK)pbis->S8Read();

            if (pwrbg->weki.wek != -1)
            {
                pwrbg->weki.sInner = pbis->F32Read();
                pwrbg->weki.uInner = pbis->F32Read();
                pwrbg->weki.sOuter = pbis->F32Read();
                pwrbg->weki.uOuter = pbis->F32Read();
                pwrbg->weki.dmat   = pbis->ReadMatrix4();
            }

            pwrbg->cmat  = pbis->U8Read();
            pwrbg->fDpos = pbis->U8Read();
            pwrbg->fDuv  = pbis->U8Read();
            pglobset->aglob[i].pwrbg = pwrbg;
            pwrbg->pwrbgNextGlobset = pglobset->pwrbgFirst;
            pglobset->pwrbgFirst = pwrbg;
        }
         
        pglobset->aglob[i].posCenter = pbis->ReadVector();
        pglobset->aglob[i].sRadius = pbis->F32Read();
        pbis->S16Read();
        pglobset->aglob[i].rtck = (RTCK)pbis->U8Read();
        pbis->U8Read();
        pglobset->aglob[i].oid = (OID)pbis->U8Read();

        if ((unk_5 & 1) == 0)
        {
            // Number of submodels
            // std::cout << "Model Start: " << std::hex << file.tellg()<<"\n";
            pglobset->aglob[i].csubglob = pbis->U16Read();
            pglobset->aglob[i].asubglob.resize(pglobset->aglob[i].csubglob);

            for (int a = 0; a < pglobset->aglob[i].csubglob; a++)
            {
                // Loading submodel origin
                pglobset->aglob[i].asubglob[a].posCenter = pbis->ReadVector();
                pglobset->aglob[i].asubglob[a].sRadius = pbis->F32Read();

                //std::cout << std::dec << "Vertex Count: " << (uint32_t)vertexCount << "\n";
                byte vertexCount = pbis->U8Read();
                //std::cout << std::dec << "Rotations Count: " << (uint32_t)rotationsCount << "\n";
                byte normalCount = pbis->U8Read();
                //std::cout << std::dec << "Vertex Color Count: " << (uint32_t)vertexColorCount << "\n";
                byte vertexColorCount = pbis->U8Read();
                //std::cout << std::dec << "Texcoords Count: " << (uint32_t)texCoordCount << "\n";
                byte texcoordCount = pbis->U8Read();
                //std::cout << std::dec << "Index Count: " << (uint32_t)indexCount << "\n";
                byte indexCount = pbis->U8Read();
                
                std::vector <glm::vec3> vertexes;
                vertexes.resize(vertexCount);

                std::vector<glm::vec3>normals;
                normals.resize(normalCount);

                std::vector<RGBA> vertexColors;
                vertexColors.resize(vertexColorCount);

                std::vector<glm::vec2> texcoords;
                texcoords.resize(texcoordCount);

                std::vector<VTXFLG> indexes;
                indexes.resize(indexCount);

                pbis->Align(4);

                //std::cout << "Vertices: " << std::hex << pbis->file.tellg() << "\n";
                for (int b = 0; b < vertexCount; b++)
                    vertexes[b] = pbis->ReadVector();

                //std::cout << "Normals: " << std::hex << pbis->file.tellg() << "\n";
                for (int c = 0; c < normalCount; c++)
                    normals[c] = pbis->ReadVector();
                
                //std::cout << "Vertex Colors: " << std::hex << pbis->file.tellg() << "\n";
                for (int d = 0; d < vertexColorCount; d++)
                {
                    vertexColors[d].bRed   = pbis->U8Read();
                    vertexColors[d].bGreen = pbis->U8Read();
                    vertexColors[d].bBlue  = pbis->U8Read();
                    vertexColors[d].bAlpha = pbis->U8Read();
                }

                //std::cout << "Texcoords: " << std::hex << pbis->file.tellg() << "\n";
                for (int e = 0; e < texcoordCount; e++)
                {
                    texcoords[e].x = pbis->F32Read();
                    texcoords[e].y = pbis->F32Read();
                }
                
                //std::cout << "Indexes: " << std::hex << pbis->file.tellg() << "\n\n";
                for (int f = 0; f < indexCount; f++)
                {
                    indexes[f].ipos    = pbis->U8Read();
                    indexes[f].inormal = pbis->U8Read();
                    indexes[f].iuv     = pbis->U8Read();
                    indexes[f].bMisc   = pbis->U8Read();
                }

                // Loading texture property
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
                
                BuildSubGlob(pglobset, pglobset->aglob[i].asubglob[a].pshd, pglobset->aglob[i].asubglob[a].vertices, vertexes, normals, vertexColors, texcoords, indexes, pglobset->aglob[i].asubglob[a].indices);
                MakeGLBuffers(&pglobset->aglob[i].asubglob[a]);
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

void BuildSubGlob(GLOBSET* pglobset, SHD* pshd, std::vector<VERTICE>& vertices, std::vector <glm::vec3>& vertexes, std::vector <glm::vec3>& normals, std::vector <RGBA>& vertexColors, std::vector <glm::vec2>& texcoords, std::vector <VTXFLG>& indexes, std::vector<uint16_t>& indices)
{
    for (int i = 0; i < indexes.size(); i++)
    {
        VERTICE vertice;

        vertice.pos    = vertexes[indexes[i].ipos];
        vertice.normal = normals[indexes[i].inormal];
        vertice.color  = (RGBA)NULL;

        if (indexes[i].iuv == 0xFF)
            vertice.uv = glm::vec2{0.0};
        else
            vertice.uv = texcoords[indexes[i].iuv];

        vertices.push_back(vertice);
    }

    for (int i = 0; i < vertexColors.size(); i++)
        vertices[i].color = vertexColors[i];

    uint32_t idx = 0;
    for (int i = 2; i < indexes.size(); i++)
    {
        byte stripFlag = indexes[idx + 2].bMisc;

        if (!(stripFlag & 0x80))
        {
            indices.push_back(idx + 0);
            indices.push_back(idx + 1);
            indices.push_back(idx + 2);
        }

        idx++;
    }
}

void MakeGLBuffers(SUBGLOB *subglob)
{
    glGenVertexArrays(1, &subglob->VAO);
    glBindVertexArray(subglob->VAO);

    glGenBuffers(1, &subglob->VBO);
    glBindBuffer(GL_ARRAY_BUFFER, subglob->VBO);
    glBufferData(GL_ARRAY_BUFFER, subglob->vertices.size() * sizeof(VERTICE), subglob->vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &subglob->EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, subglob->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, subglob->indices.size() * sizeof(uint16_t), subglob->indices.data(), GL_STATIC_DRAW);

    // Vertex Position's
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal's
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)12);
    glEnableVertexAttribArray(1);

    // Vertex Color's
    glVertexAttribPointer(2, 1, GL_UNSIGNED_INT, GL_FALSE, sizeof(VERTICE), (void*)24);
    glEnableVertexAttribArray(2);

    // Uv's
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)28);
    glEnableVertexAttribArray(3);
}