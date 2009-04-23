using System;
using System.IO;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace Framework1.Quake3
{
    using BspFile;

    public class BspLoader
    {
        string m_Path;
        Stream m_Stream;
        BinaryReader m_Reader;
        byte[] m_Buffer;

        public BspLoader(string path)
        {
            m_Path = path;
            try
            {
                m_Stream = File.OpenRead(m_Path);
                m_Reader = new BinaryReader(m_Stream);
                m_Buffer = new byte[128];
            }
            catch
            {
                m_Stream = null;
            }
        }

        public Header Open()
        {
            if (m_Stream == null)
                return null;

            Type type = typeof(Header.Binary);
            int size = Marshal.SizeOf(type);

            if (m_Buffer.Length < size)
            {
                m_Buffer = new byte[size];
            }

            m_Stream.Seek(0, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, size) == size)
            {
                Header.Binary binary = (Header.Binary)BinarySerializer.RawDeserialize(m_Buffer, 0, type);

                Header asset = new Header();
                asset.Header = asset;
                asset.Loader = this;
                asset.StreamOffset = 0;

                if (asset.construct(binary))
                    return asset;
            }

            return null;
        }

        public int GetFaceCount(Header header)
        {
            Type type = typeof(Faces.Binary_face);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Faces)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public Faces GetFaces(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(Faces.Binary_face);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Faces)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            Faces.Binary_face[] binaryFaces = new Faces.Binary_face[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryFaces[i] = (Faces.Binary_face)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                Faces asset = new Faces();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryFaces))
                    return asset;
            }

            return null;
        }

        public int GetVertexCount(Header header)
        {
            Type type = typeof(Vertices.Binary_vertex);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Vertices)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public Vertices GetVertices(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(Vertices.Binary_vertex);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Vertices)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            Vertices.Binary_vertex[] binaryVertices = new Vertices.Binary_vertex[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryVertices[i] = (Vertices.Binary_vertex)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                Vertices asset = new Vertices();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryVertices))
                    return asset;
            }

            return null;
        }

        public int GetMeshVertexCount(Header header)
        {
            Type type = typeof(MeshVertices.Binary_meshvert);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Meshverts)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public MeshVertices GetMeshVertices(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(MeshVertices.Binary_meshvert);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Meshverts)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            MeshVertices.Binary_meshvert[] binaryVertices = new MeshVertices.Binary_meshvert[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryVertices[i] = (MeshVertices.Binary_meshvert)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                MeshVertices asset = new MeshVertices();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryVertices))
                    return asset;
            }

            return null;
        }

        public int GetNodeCount(Header header)
        {
            Type type = typeof(Nodes.Binary_node);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Nodes)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public Nodes GetNodes(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(Nodes.Binary_node);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Nodes)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            Nodes.Binary_node[] binaryNodes = new Nodes.Binary_node[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryNodes[i] = (Nodes.Binary_node)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                Nodes asset = new Nodes();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryNodes))
                    return asset;
            }

            return null;
        }

        public int GetLeafCount(Header header)
        {
            Type type = typeof(Leafs.Binary_leaf);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Leafs)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public Leafs GetLeafs(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(Leafs.Binary_leaf);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Leafs)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            Leafs.Binary_leaf[] binaryLeafs = new Leafs.Binary_leaf[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryLeafs[i] = (Leafs.Binary_leaf)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                Leafs asset = new Leafs();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryLeafs))
                    return asset;
            }

            return null;
        }

        public int GetLeafFaceCount(Header header)
        {
            Type type = typeof(LeafFaces.Binary_leafface);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Leaffaces)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public LeafFaces GetLeafFaces(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(LeafFaces.Binary_leafface);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Leaffaces)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            LeafFaces.Binary_leafface[] binaryLeafFaces = new LeafFaces.Binary_leafface[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryLeafFaces[i] = (LeafFaces.Binary_leafface)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                LeafFaces asset = new LeafFaces();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryLeafFaces))
                    return asset;
            }

            return null;
        }

        public int GetPlaneCount(Header header)
        {
            Type type = typeof(Planes.Binary_plane);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Planes)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public Planes GetPlanes(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(Planes.Binary_plane);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Planes)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            Planes.Binary_plane[] binaryPlanes = new Planes.Binary_plane[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryPlanes[i] = (Planes.Binary_plane)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                Planes asset = new Planes();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryPlanes))
                    return asset;
            }

            return null;
        }

        public bool HasVisdata(Header header)
        {
            if (header == null)
                return false;

            if (header.m_DirEntries[(int)(Header.EntityType.Visdata)].length == 0)
                return false;

            Type infoType = typeof(Visdata.Binary_info);
            int infoSize = Marshal.SizeOf(infoType);

            Type type = typeof(Visdata.Binary_data);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Visdata)].offset;
            long dataSize = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Visdata)].length;
            long startOffset = baseOffset + infoSize;
            long endOffset = startOffset + (dataSize - infoSize);
            int totalSize = (int)(endOffset - startOffset);
            int count = totalSize / size;

            return count > 0;
        }

        public Visdata GetVisdata(Header header)
        {
            if (header == null)
                return null;

            if (header.m_DirEntries[(int)(Header.EntityType.Visdata)].length == 0)
                return null;

            Type infoType = typeof(Visdata.Binary_info);
            int infoSize = Marshal.SizeOf(infoType);

            Type type = typeof(Visdata.Binary_data);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Visdata)].offset;
            long dataSize = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Visdata)].length;
            long startOffset = baseOffset + infoSize;
            long endOffset = startOffset + (dataSize - infoSize);
            int totalSize = (int)(endOffset - startOffset);
            int count = totalSize / size;

            if (count == 0)
                return null;

            Trace.Assert(count * size == totalSize);

            Visdata.Binary_info binaryInfo = new Visdata.Binary_info();

            {
                if (m_Buffer.Length < infoSize)
                {
                    m_Buffer = new byte[infoSize];
                }

                m_Stream.Seek(baseOffset, SeekOrigin.Begin);
                if (m_Stream.Read(m_Buffer, 0, infoSize) == infoSize)
                {
                    binaryInfo = (Visdata.Binary_info)BinarySerializer.RawDeserialize(m_Buffer, 0, infoType);

                    Trace.Assert(binaryInfo.n_vecs * binaryInfo.sz_vecs == count);
                }
                else
                {
                    return null;
                }
            }

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            Visdata.Binary_data[] binaryVisdata = new Visdata.Binary_data[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryVisdata[i] = (Visdata.Binary_data)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                Visdata asset = new Visdata();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryInfo, binaryVisdata))
                    return asset;
            }

            return null;
        }

        public int GetTextureCount(Header header)
        {
            Type type = typeof(Textures.Binary_texture);
            int size = Marshal.SizeOf(type);
            int length = header.m_DirEntries[(int)(Header.EntityType.Textures)].length;
            int count = (length / size);

            Trace.Assert(count * size == length);

            return count;
        }

        public Textures GetTextures(Header header, int startIndex, int count)
        {
            if (header == null)
                return null;

            Type type = typeof(Textures.Binary_texture);
            int size = Marshal.SizeOf(type);

            long baseOffset = header.StreamOffset + header.m_DirEntries[(int)(Header.EntityType.Textures)].offset;
            long startOffset = baseOffset + startIndex * size;
            long endOffset = startOffset + count * size;
            int totalSize = (int)(endOffset - startOffset);

            if (m_Buffer.Length < totalSize)
            {
                m_Buffer = new byte[totalSize];
            }

            Textures.Binary_texture[] binaryTextures = new Textures.Binary_texture[count];

            m_Stream.Seek(startOffset, SeekOrigin.Begin);
            if (m_Stream.Read(m_Buffer, 0, totalSize) == totalSize)
            {
                int offset = 0;
                for (int i = 0; i < count; ++i, offset += size)
                {
                    binaryTextures[i] = (Textures.Binary_texture)BinarySerializer.RawDeserialize(m_Buffer, offset, type);
                }

                Textures asset = new Textures();
                asset.StreamOffset = startOffset;
                asset.Header = header;

                if (asset.construct(binaryTextures))
                    return asset;
            }

            return null;
        }
    }

    class BinarySerializer
    {
        public static byte[] RawSerialize(object anything)
        {
            int rawsize = Marshal.SizeOf(anything);
            IntPtr buffer = Marshal.AllocHGlobal(rawsize);
            Marshal.StructureToPtr(anything, buffer, false);
            byte[] rawdatas = new byte[rawsize];
            Marshal.Copy(buffer, rawdatas, 0, rawsize);
            Marshal.FreeHGlobal(buffer);
            return rawdatas;

        }


        public static object RawDeserialize(byte[] rawdatas, int offset, Type anytype)
        {
            int rawsize = Marshal.SizeOf(anytype);
            if (rawsize > rawdatas.Length)
                return null;
            IntPtr buffer = Marshal.AllocHGlobal(rawsize);
            Marshal.Copy(rawdatas, offset, buffer, rawsize);
            object retobj = Marshal.PtrToStructure(buffer, anytype);
            Marshal.FreeHGlobal(buffer);
            return retobj;

        }
    }
}