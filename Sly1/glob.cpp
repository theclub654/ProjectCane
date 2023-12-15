#include "glob.h"

std::vector <SHD> g_ashd;
extern std::vector<ALO*> allSWAloObjs;
extern std::vector<void*> allSwLights;

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
            pglobset->aglob[i].pdmat =
            {
                pbis->F32Read(), pbis->F32Read(),pbis->F32Read(),0.0f,
                pbis->F32Read(), pbis->F32Read(),pbis->F32Read(),0.0f,
                pbis->F32Read(), pbis->F32Read(),pbis->F32Read(),0.0f,
                pbis->F32Read(), pbis->F32Read(),pbis->F32Read(),1.0f,
            };
        }

        else
            pglobset->aglob[i].pdmat = glm::identity<glm::mat4>();

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
                pbis->ReadMatrix4x4();
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
                
                pglobset->aglob[i].asubglob[a].vertexes.resize(vertexCount);
                pglobset->aglob[i].asubglob[a].normals.resize(normalCount);
                pglobset->aglob[i].asubglob[a].vertexColors.resize(vertexColorCount);
                pglobset->aglob[i].asubglob[a].texcoords.resize(texcoordCount);
                pglobset->aglob[i].asubglob[a].indexes.resize(indexCount);

                pbis->Align(4);
                
                //std::cout << "Vertices: " << std::hex << pbis->file.tellg() << "\n";
                for (int b = 0; b < vertexCount; b++)
                {
                    pglobset->aglob[i].asubglob[a].vertexes[b] = pbis->ReadVector();
                }
                 
                //std::cout << "Normals: " << std::hex << pbis->file.tellg() << "\n";
                for (int c = 0; c < normalCount; c++)
                    pglobset->aglob[i].asubglob[a].normals[c] = pbis->ReadVector();

                //std::cout << "Vertex Colors: " << std::hex << pbis->file.tellg() << "\n";
                for (int d = 0; d < vertexColorCount; d++)
                {
                    pglobset->aglob[i].asubglob[a].vertexColors[d].bRed = pbis->U8Read();
                    pglobset->aglob[i].asubglob[a].vertexColors[d].bGreen = pbis->U8Read();
                    pglobset->aglob[i].asubglob[a].vertexColors[d].bBlue = pbis->U8Read();
                    pglobset->aglob[i].asubglob[a].vertexColors[d].bAlpha = pbis->U8Read();
                }

                //std::cout << "Texcoords: " << std::hex << pbis->file.tellg() << "\n";
                for (int e = 0; e < texcoordCount; e++)
                {
                    pglobset->aglob[i].asubglob[a].texcoords[e].x = pbis->F32Read();
                    pglobset->aglob[i].asubglob[a].texcoords[e].y = pbis->F32Read();
                }

                //std::cout << "Indexes: " << std::hex << pbis->file.tellg() << "\n\n";
                for (int f = 0; f < indexCount; f++)
                {
                    pglobset->aglob[i].asubglob[a].indexes[f].ipos = pbis->U8Read();
                    pglobset->aglob[i].asubglob[a].indexes[f].inormal = pbis->U8Read();
                    pglobset->aglob[i].asubglob[a].indexes[f].iuv = pbis->U8Read();
                    pglobset->aglob[i].asubglob[a].indexes[f].bMisc = pbis->U8Read();
                }
                
                // Loading texture property info from vector
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
                
                BuildSubGlob(pglobset, pglobset->aglob[i].asubglob[a].pshd, pglobset->aglob[i].asubglob[a].vertices, pglobset->aglob[i].asubglob[a].vertexes, pglobset->aglob[i].asubglob[a].normals, pglobset->aglob[i].asubglob[a].vertexColors, pglobset->aglob[i].asubglob[a].texcoords, pglobset->aglob[i].asubglob[a].indexes, pglobset->aglob[i].asubglob[a].indices);
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

    if (pglobset->cglob != 0)
    {
        allSWAloObjs.push_back(palo);
        MakeGLBuffers(pglobset);
    }
}

void BuildSubGlob(GLOBSET* pglobset, SHD* pshd ,std::vector<VERTICE>& vertices,std::vector<glm::vec3>& vertexes, std::vector<glm::vec3>& normals, std::vector<RGBA>& vertexColors, std::vector<glm::vec2>& texcoords, std::vector<VTXFLG>& indexes, std::vector<uint16_t> &indices)
{
    for (int i = 0; i < indexes.size(); i++)
    {
        VERTICE vertice;

        vertice.pos = vertexes[indexes[i].ipos];

        if (indexes[i].iuv == 0xFF)
            vertice.uv = glm::vec2{0.0};
        else
            vertice.uv = texcoords[indexes[i].iuv];

        vertices.push_back(vertice);
    }

    uint32_t idx = 0;
    for (int i = 2; i < indexes.size(); i++)
    {
        uint16_t indice0 = indexes[idx + 0].ipos;
        uint16_t indice1 = indexes[idx + 1].ipos;
        uint16_t indice2 = indexes[idx + 2].ipos;
        byte stripFlag   = indexes[idx + 2].bMisc;

        if (!(stripFlag & 0x80))
        {
            indices.push_back(idx + 0);
            indices.push_back(idx + 1);
            indices.push_back(idx + 2);
        }

        idx++;
    }
}

void MakeGLBuffers(GLOBSET *pglobset)
{
    for (int i = 0; i < pglobset->aglob.size(); i++)
    {
        for (int a = 0; a < pglobset->aglob[i].asubglob.size(); a++)
        {
            glGenVertexArrays(1, &pglobset->aglob[i].asubglob[a].VAO);
            glBindVertexArray(pglobset->aglob[i].asubglob[a].VAO);

            glGenBuffers(1, &pglobset->aglob[i].asubglob[a].VBO);
            glBindBuffer(GL_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].VBO);
            glBufferData(GL_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].vertices.size() * sizeof(VERTICE), pglobset->aglob[i].asubglob[a].vertices.data(), GL_STATIC_DRAW);

            glGenBuffers(1, &pglobset->aglob[i].asubglob[a].EBO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, pglobset->aglob[i].asubglob[a].indices.size() * sizeof(uint16_t), pglobset->aglob[i].asubglob[a].indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)12);
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VERTICE), (void*)24);
            glEnableVertexAttribArray(2);

            glBindVertexArray(0);
        }
    }
}

void DrawGlob(GLOBSET* pglobset, glm::vec3 pos)
{
    glShader.Use();

    for (int i = 0; i < pglobset->cglob; i++)
    {
        for (int a = 0; a < pglobset->aglob[i].csubglob; a++)
        {
            glm::mat4 model{ 1.0 };
            
            int modelUniformLocation = glGetUniformLocation(glShader.ID, "model");
            glUniformMatrix4fv(modelUniformLocation, 1, GL_FALSE, glm::value_ptr(model));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, pglobset->aglob[i].asubglob[a].pshd->glTexture);

            glBindVertexArray(pglobset->aglob[i].asubglob[a].VAO);
            glDrawElements(GL_TRIANGLES, pglobset->aglob[i].asubglob[a].indices.size(), GL_UNSIGNED_SHORT, (void*)0);
            glBindVertexArray(0);
        }
    }
}