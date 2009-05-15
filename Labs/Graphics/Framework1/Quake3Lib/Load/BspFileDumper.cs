using System;
using System.IO;
using System.Diagnostics;

namespace Framework1.Quake3
{
    class BspFileDumper : IDisposable
    {
        BspFile.Header Header;
        string FilePath;

        public BspFileDumper(BspFile.Header header)
        {
            Header = header;
        }

        public BspFileDumper(string filePath)
        {
            FilePath = filePath;
            BspLoader loader = new BspLoader(filePath);
            Header = loader.Open();
        }

        public void Dispose()
        {
        }

        public void Print(bool dumpTextures, bool dumpEffects)
        {
            Console.WriteLine("> Start bsp file dump");

            if (FilePath != null)
            {
                Console.WriteLine(FilePath);
            }

            {
                int sizeOffset = 0;

                for (int i = 0; i < (int)BspFile.Header.EntityType.Count; ++i)
                {
                    if (Header.m_DirEntries[i].offset + Header.m_DirEntries[i].length > sizeOffset)
                    {
                        sizeOffset = Header.m_DirEntries[i].offset + Header.m_DirEntries[i].length;
                    }
                }
                Console.WriteLine(string.Format("{0:G} file bytes", sizeOffset));
            }

            {
                int faceCount = Header.Loader.GetFaceCount(Header);

                Console.WriteLine(string.Format("{0:G} faces", faceCount));

                /*
                if (faceCount > 0)
                {
                    using (BspFile.Faces faces = Header.Loader.GetFaces(Header, 0, faceCount))
                    {
                        Trace.Assert(faces != null);
                    }
                }
                */
            }

            {
                int vertexCount = Header.Loader.GetVertexCount(Header);

                Console.WriteLine(string.Format("{0:G} vertices", vertexCount));

                /*
                if (vertexCount > 0)
                {
                    BspFile.Vertices vertices = Header.Loader.GetVertices(Header, 0, vertexCount);
                    Trace.Assert(vertices != null);
                }
                */ 
            }

            {
                int planeCount = Header.Loader.GetPlaneCount(Header);

                Console.WriteLine(string.Format("{0:G} planes", planeCount));
            }

            {
                int leafCount = Header.Loader.GetLeafCount(Header);

                Console.WriteLine(string.Format("{0:G} leafs", leafCount));
            }

            {
                int leafFaceCount = Header.Loader.GetLeafFaceCount(Header);

                Console.WriteLine(string.Format("{0:G} leafFaces", leafFaceCount));
            }

            {
                bool hasVisData = Header.Loader.HasVisdata(Header);

                if (hasVisData)
                    Console.WriteLine("Visdata yes");
                else
                    Console.WriteLine("Visdata no");
            }

            {
                int textureCount = Header.Loader.GetTextureCount(Header);

                Console.WriteLine(string.Format("{0:G} textures", textureCount));

                if (textureCount > 0 && dumpTextures)
                {
                    Console.WriteLine("> Start texture names");

                    using (BspFile.Textures textures = Header.Loader.GetTextures(Header, 0, textureCount))
                    {
                        Trace.Assert(textures != null);

                        for (int i = 0; i < textures.m_Textures.Length; ++i)
                        {
                            Console.WriteLine(textures.m_Textures[i].GetTextureNameString());
                        }
                    }

                    Console.WriteLine("> End texture names");
                }
            }

            {
                int lightmapCount = Header.Loader.GetLightmapCount(Header);

                Console.WriteLine(string.Format("{0:G} lightmaps", lightmapCount));
            }

            {
                int effectCount = Header.Loader.GetEffectCount(Header);

                Console.WriteLine(string.Format("{0:G} effects", effectCount));

                if (effectCount > 0 && dumpEffects)
                {
                    Console.WriteLine("> Start effect names");

                    using (BspFile.Effects effects = Header.Loader.GetEffects(Header, 0, effectCount))
                    {
                        Trace.Assert(effects != null);

                        for (int i = 0; i < effects.m_Effects.Length; ++i)
                        {
                            Console.WriteLine(effects.m_Effects[i].GetEffectNameString());
                        }
                    }

                    Console.WriteLine("> End effect names");
                }
            }

            Console.WriteLine("> End bsp file dump");
        }
    }
}