using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework;

namespace Framework1.Quake3.BspFile
{
    public enum FaceType
    {
        None = 0, Polygon = 1, Patch = 2, Mesh = 3, Billboard = 4
    }

    public class CoordSysConv
    {
        public void ConvertFace<T>(ref T i0, ref T i1, ref T i2) 
        {
            //T i = i1;
            //i1 = i2;
            //i2 = i;
        }
        
        public void ConvertVector(ref Vector3 vector) 
        {
            float temp = vector.Z;

            vector.Z = -vector.Y;
            vector.Y = temp;
        }
        
        public void ConvertTexCoord(ref Vector2 vector) { }
    }

    public class Asset
    {
        public Header Header;
        public long StreamOffset;
    }

    public class Header : Asset
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_direnty
        {
            public int offset;
            public int length;
        }

        [StructLayout(LayoutKind.Sequential)]
        public class Binary
        {
            public char magic0;
            public char magic1;
            public char magic2;
            public char magic3;
            public int version;
            public Binary_direnty Entities;
            public Binary_direnty Textures;
            public Binary_direnty Planes;
            public Binary_direnty Nodes;
            public Binary_direnty Leafs;
            public Binary_direnty Leaffaces;
            public Binary_direnty Leafbrushes;
            public Binary_direnty Models;
            public Binary_direnty Brushes;
            public Binary_direnty Brushsides;
            public Binary_direnty Vertices;
            public Binary_direnty Meshverts;
            public Binary_direnty Effects;
            public Binary_direnty Faces;
            public Binary_direnty Lightmaps;
            public Binary_direnty Lightvols;
            public Binary_direnty Visdata;
        }

        public enum EntityType
        {
            Entities = 0, Textures, Planes, Nodes, Leafs, Leaffaces, Leafbrushes,
            Models, Brushes, Brushsides, Vertices, Meshverts,
            Effects, Faces, Lightmaps, Lightvols, Visdata, Count
        }

        public BspLoader Loader;
        public Binary_direnty[] m_DirEntries;

        public bool construct(Binary binary)
        {
            if (binary.magic0 == 'I' && binary.magic1 == 'B'
                && binary.magic2 == 'S' && binary.magic3 == 'P'
                && binary.version == 0x2e)
            {
                m_DirEntries = new Binary_direnty[(int)EntityType.Count];

                foreach (EntityType type in Enum.GetValues(typeof(EntityType)))
                {
                    if (type != EntityType.Count)
                    {
                        m_DirEntries[(int)type] = (Binary_direnty)binary.GetType().GetField(type.ToString()).GetValue(binary);
                    }
                }

                return true;
            }

            return false;
        }
    }

    public class Faces : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_face
        {
            public int texture; 	//Texture index.
            public int effect; 	//Index into lump 12 (Effects), or -1.
            public int type; 	//Face type. 1=polygon, 2=patch, 3=mesh, 4=billboard
            public int vertex; 	//Index of first vertex.
            public int n_vertexes; 	//Number of vertices.
            public int meshvert; 	//Index of first meshvert.
            public int n_meshverts; 	//Number of meshverts.
            public int lm_index;	//Lightmap index.
            public int lm_start_s;	//Corner of this face's lightmap image in lightmap.
            public int lm_start_t;
            public int lm_size_s; 	//Size of this face's lightmap image in lightmap.
            public int lm_size_t;
            public float lm_origin_x; 	//World space origin of lightmap.
            public float lm_origin_y;
            public float lm_origin_z;
            public float lm_vecs0_s; //World space lightmap s and t unit vectors.
            public float lm_vecs0_t;
            public float lm_vecs1_s;
            public float lm_vecs1_t;
            public float lm_vecs2_s;
            public float lm_vecs2_t;
            public float normal_x; 	//Surface normal.
            public float normal_y;
            public float normal_z;
            public int size_x; 	//Patch dimensions. 
            public int size_y;
        }

        public Binary_face[] m_Faces;

        public bool construct(Binary_face[] faces)
        {
            m_Faces = faces;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Vertices : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_vertex
        {
            public float position_x; 	//Vertex position.
            public float position_y;
            public float position_z;
            public float texcoord_s; 	//Vertex texture coordinates. 0=surface, 1=lightmap.
            public float texcoord_t;
            public float texcoord_lm_s;
            public float texcoord_lm_t;
            public float normal_x; 	//Vertex normal.
            public float normal_y;
            public float normal_z;
            public byte color_r;
            public byte color_g;
            public byte color_b;
            public byte color_a;
        }

        public Binary_vertex[] m_Vertices;

        public bool construct(Binary_vertex[] vertices)
        {
            m_Vertices = vertices;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class MeshVertices : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_meshvert
        {
            public int offset;
        }

        public Binary_meshvert[] m_Vertices;

        public bool construct(Binary_meshvert[] vertices)
        {
            m_Vertices = vertices;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Nodes : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_node
        {
            public int plane; 	    //Plane index.
            public int child_0_front; 	// Children indices. Negative numbers are leaf indices: -(leaf+1).
            public int child_1_back;
            public int min_0;       //Integer bounding box min coord.
            public int min_1;
            public int min_2;
            public int max_0;       //Integer bounding box max coord. 
            public int max_1;
            public int max_2;
        }

        public Binary_node[] m_Nodes;

        public bool construct(Binary_node[] nodes)
        {
            m_Nodes = nodes;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Leafs : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_leaf
        {
           public int cluster; 	        // visdata cluster index.
           public int area; 	        // Areaportal area.
           public int min_0; 	        // Integer bounding box min coord.
           public int min_1;
           public int min_2;
           public int max_0; 	        // Integer bounding box max coord.
           public int max_1;
           public int max_2;
           public int leafface; 	        // First leafface for leaf.
           public int n_leaffaces; 	    // Number of leaffaces for leaf.
           public int leafbrush; 	    // First leafbrush for leaf.
           public int n_leafbrushes; 	// Number of leafbrushes for leaf. 
        }

        public Binary_leaf[] m_Leafs;

        public bool construct(Binary_leaf[] leafs)
        {
            m_Leafs = leafs;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class LeafFaces : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_leafface
        {
            public int face;
        }

        public Binary_leafface[] m_LeafFaces;

        public bool construct(Binary_leafface[] leaffaces)
        {
            m_LeafFaces = leaffaces;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Planes : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_plane
        {
            public float normal_0; 	    // Plane normal. 
            public float normal_1;
            public float normal_2;
            public float dist; 	        // Distance from origin to plane along normal. 
            
        }

        public Binary_plane[] m_Planes;

        public bool construct(Binary_plane[] planes)
        {
            m_Planes = planes;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Visdata : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_info
        {
            public int n_vecs; 	   	// Number of vectors. 
            public int sz_vecs;     // 	Size of each vector, in bytes. 
        }

        [StructLayout(LayoutKind.Sequential)]
        public class Binary_data
        {
            public byte data; 	   	
        }

        public Binary_info m_Info;
        public Binary_data[] m_Data;

        public bool construct(Binary_info info, Binary_data[] data)
        {
            m_Info = info;
            m_Data = data;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Textures : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_texture
        {
            public sbyte name_0; 	    // 	Texture name. 
            public sbyte name_1;public sbyte name_2;public sbyte name_3;public sbyte name_4;public sbyte name_5;public sbyte name_6;public sbyte name_7;
            public sbyte name_8;public sbyte name_9;public sbyte name_10;public sbyte name_11;public sbyte name_12;public sbyte name_13;public sbyte name_14;
            public sbyte name_15;public sbyte name_16;public sbyte name_17;public sbyte name_18;public sbyte name_19;public sbyte name_20;public sbyte name_21;
            public sbyte name_22;public sbyte name_23;public sbyte name_24;public sbyte name_25;public sbyte name_26;public sbyte name_27;public sbyte name_28;
            public sbyte name_29;public sbyte name_30;public sbyte name_31;public sbyte name_32;public sbyte name_33;public sbyte name_34;public sbyte name_35;
            public sbyte name_36;public sbyte name_37;public sbyte name_38;public sbyte name_39;public sbyte name_40;public sbyte name_41;public sbyte name_42;
            public sbyte name_43;public sbyte name_44;public sbyte name_45;public sbyte name_46;public sbyte name_47;public sbyte name_48;public sbyte name_49;
            public sbyte name_50;public sbyte name_51;public sbyte name_52;public sbyte name_53;public sbyte name_54;public sbyte name_55;public sbyte name_56;
            public sbyte name_57;public sbyte name_58;public sbyte name_59;public sbyte name_60;public sbyte name_61;public sbyte name_62;public sbyte name_63;
            
            public int flags;           // 	Surface flags. 
            public int contents;        // 	Content flags. 

            public string GetTextureNameString()
            {
                StringBuilder textureName = new StringBuilder(64, 64);

                for (int i = 0; i < 1; ++i)
                {
                    if (name_0 > 0) textureName.Append((char)name_0); else break;
                    if (name_1 > 0) textureName.Append((char)name_1); else break;
                    if (name_2 > 0) textureName.Append((char)name_2); else break;
                    if (name_3 > 0) textureName.Append((char)name_3); else break;
                    if (name_4 > 0) textureName.Append((char)name_4); else break;
                    if (name_5 > 0) textureName.Append((char)name_5); else break;
                    if (name_6 > 0) textureName.Append((char)name_6); else break;
                    if (name_7 > 0) textureName.Append((char)name_7); else break;
                    if (name_8 > 0) textureName.Append((char)name_8); else break;
                    if (name_9 > 0) textureName.Append((char)name_9); else break;
                    if (name_10 > 0) textureName.Append((char)name_10); else break;
                    if (name_11 > 0) textureName.Append((char)name_11); else break;
                    if (name_12 > 0) textureName.Append((char)name_12); else break;
                    if (name_13 > 0) textureName.Append((char)name_13); else break;
                    if (name_14 > 0) textureName.Append((char)name_14); else break;
                    if (name_15 > 0) textureName.Append((char)name_15); else break;
                    if (name_16 > 0) textureName.Append((char)name_16); else break;
                    if (name_17 > 0) textureName.Append((char)name_17); else break;
                    if (name_18 > 0) textureName.Append((char)name_18); else break;
                    if (name_19 > 0) textureName.Append((char)name_19); else break;
                    if (name_20 > 0) textureName.Append((char)name_20); else break;
                    if (name_21 > 0) textureName.Append((char)name_21); else break;
                    if (name_22 > 0) textureName.Append((char)name_22); else break;
                    if (name_23 > 0) textureName.Append((char)name_23); else break;
                    if (name_24 > 0) textureName.Append((char)name_24); else break;
                    if (name_25 > 0) textureName.Append((char)name_25); else break;
                    if (name_26 > 0) textureName.Append((char)name_26); else break;
                    if (name_27 > 0) textureName.Append((char)name_27); else break;
                    if (name_28 > 0) textureName.Append((char)name_28); else break;
                    if (name_29 > 0) textureName.Append((char)name_29); else break;
                    if (name_30 > 0) textureName.Append((char)name_30); else break;
                    if (name_31 > 0) textureName.Append((char)name_31); else break;
                    if (name_32 > 0) textureName.Append((char)name_32); else break;
                    if (name_33 > 0) textureName.Append((char)name_33); else break;
                    if (name_34 > 0) textureName.Append((char)name_34); else break;
                    if (name_35 > 0) textureName.Append((char)name_35); else break;
                    if (name_36 > 0) textureName.Append((char)name_36); else break;
                    if (name_37 > 0) textureName.Append((char)name_37); else break;
                    if (name_38 > 0) textureName.Append((char)name_38); else break;
                    if (name_39 > 0) textureName.Append((char)name_39); else break;
                    if (name_40 > 0) textureName.Append((char)name_40); else break;
                    if (name_41 > 0) textureName.Append((char)name_41); else break;
                    if (name_42 > 0) textureName.Append((char)name_42); else break;
                    if (name_43 > 0) textureName.Append((char)name_43); else break;
                    if (name_44 > 0) textureName.Append((char)name_44); else break;
                    if (name_45 > 0) textureName.Append((char)name_45); else break;
                    if (name_46 > 0) textureName.Append((char)name_46); else break;
                    if (name_47 > 0) textureName.Append((char)name_47); else break;
                    if (name_48 > 0) textureName.Append((char)name_48); else break;
                    if (name_49 > 0) textureName.Append((char)name_49); else break;
                    if (name_50 > 0) textureName.Append((char)name_50); else break;
                    if (name_51 > 0) textureName.Append((char)name_51); else break;
                    if (name_52 > 0) textureName.Append((char)name_52); else break;
                    if (name_53 > 0) textureName.Append((char)name_53); else break;
                    if (name_54 > 0) textureName.Append((char)name_54); else break;
                    if (name_55 > 0) textureName.Append((char)name_55); else break;
                    if (name_56 > 0) textureName.Append((char)name_56); else break;
                    if (name_57 > 0) textureName.Append((char)name_57); else break;
                    if (name_58 > 0) textureName.Append((char)name_58); else break;
                    if (name_59 > 0) textureName.Append((char)name_59); else break;
                    if (name_60 > 0) textureName.Append((char)name_60); else break;
                    if (name_61 > 0) textureName.Append((char)name_61); else break;
                    if (name_62 > 0) textureName.Append((char)name_62); else break;
                    if (name_63 > 0) textureName.Append((char)name_63); else break;
                }

                return textureName.ToString();
            }
        }

        public Binary_texture[] m_Textures;

        public bool construct(Binary_texture[] textures)
        {
            m_Textures = textures;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Lightmaps : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_lightmap
        {
            public Microsoft.Xna.Framework.Graphics.Color[] pixels;
        }

        public Binary_lightmap[] m_Lightmaps;

        public bool construct(Binary_lightmap[] lightmaps)
        {
            m_Lightmaps = lightmaps;
            return true;
        }

        public void Dispose()
        {
        }
    }

    public class Effects : Asset, IDisposable
    {
        [StructLayout(LayoutKind.Sequential)]
        public class Binary_effect
        {
            public sbyte name_0; 	    // 	Texture name. 
            public sbyte name_1; public sbyte name_2; public sbyte name_3; public sbyte name_4; public sbyte name_5; public sbyte name_6; public sbyte name_7;
            public sbyte name_8; public sbyte name_9; public sbyte name_10; public sbyte name_11; public sbyte name_12; public sbyte name_13; public sbyte name_14;
            public sbyte name_15; public sbyte name_16; public sbyte name_17; public sbyte name_18; public sbyte name_19; public sbyte name_20; public sbyte name_21;
            public sbyte name_22; public sbyte name_23; public sbyte name_24; public sbyte name_25; public sbyte name_26; public sbyte name_27; public sbyte name_28;
            public sbyte name_29; public sbyte name_30; public sbyte name_31; public sbyte name_32; public sbyte name_33; public sbyte name_34; public sbyte name_35;
            public sbyte name_36; public sbyte name_37; public sbyte name_38; public sbyte name_39; public sbyte name_40; public sbyte name_41; public sbyte name_42;
            public sbyte name_43; public sbyte name_44; public sbyte name_45; public sbyte name_46; public sbyte name_47; public sbyte name_48; public sbyte name_49;
            public sbyte name_50; public sbyte name_51; public sbyte name_52; public sbyte name_53; public sbyte name_54; public sbyte name_55; public sbyte name_56;
            public sbyte name_57; public sbyte name_58; public sbyte name_59; public sbyte name_60; public sbyte name_61; public sbyte name_62; public sbyte name_63;

            public int brush;           // 	Brush that generated this effect. 
            public int unknown;        // Always 5, except in q3dm8, which has one effect with -1. 

            public string GetEffectNameString()
            {
                StringBuilder effectName = new StringBuilder(64, 64);

                for (int i = 0; i < 1; ++i)
                {
                    if (name_0 > 0) effectName.Append((char)name_0); else break;
                    if (name_1 > 0) effectName.Append((char)name_1); else break;
                    if (name_2 > 0) effectName.Append((char)name_2); else break;
                    if (name_3 > 0) effectName.Append((char)name_3); else break;
                    if (name_4 > 0) effectName.Append((char)name_4); else break;
                    if (name_5 > 0) effectName.Append((char)name_5); else break;
                    if (name_6 > 0) effectName.Append((char)name_6); else break;
                    if (name_7 > 0) effectName.Append((char)name_7); else break;
                    if (name_8 > 0) effectName.Append((char)name_8); else break;
                    if (name_9 > 0) effectName.Append((char)name_9); else break;
                    if (name_10 > 0) effectName.Append((char)name_10); else break;
                    if (name_11 > 0) effectName.Append((char)name_11); else break;
                    if (name_12 > 0) effectName.Append((char)name_12); else break;
                    if (name_13 > 0) effectName.Append((char)name_13); else break;
                    if (name_14 > 0) effectName.Append((char)name_14); else break;
                    if (name_15 > 0) effectName.Append((char)name_15); else break;
                    if (name_16 > 0) effectName.Append((char)name_16); else break;
                    if (name_17 > 0) effectName.Append((char)name_17); else break;
                    if (name_18 > 0) effectName.Append((char)name_18); else break;
                    if (name_19 > 0) effectName.Append((char)name_19); else break;
                    if (name_20 > 0) effectName.Append((char)name_20); else break;
                    if (name_21 > 0) effectName.Append((char)name_21); else break;
                    if (name_22 > 0) effectName.Append((char)name_22); else break;
                    if (name_23 > 0) effectName.Append((char)name_23); else break;
                    if (name_24 > 0) effectName.Append((char)name_24); else break;
                    if (name_25 > 0) effectName.Append((char)name_25); else break;
                    if (name_26 > 0) effectName.Append((char)name_26); else break;
                    if (name_27 > 0) effectName.Append((char)name_27); else break;
                    if (name_28 > 0) effectName.Append((char)name_28); else break;
                    if (name_29 > 0) effectName.Append((char)name_29); else break;
                    if (name_30 > 0) effectName.Append((char)name_30); else break;
                    if (name_31 > 0) effectName.Append((char)name_31); else break;
                    if (name_32 > 0) effectName.Append((char)name_32); else break;
                    if (name_33 > 0) effectName.Append((char)name_33); else break;
                    if (name_34 > 0) effectName.Append((char)name_34); else break;
                    if (name_35 > 0) effectName.Append((char)name_35); else break;
                    if (name_36 > 0) effectName.Append((char)name_36); else break;
                    if (name_37 > 0) effectName.Append((char)name_37); else break;
                    if (name_38 > 0) effectName.Append((char)name_38); else break;
                    if (name_39 > 0) effectName.Append((char)name_39); else break;
                    if (name_40 > 0) effectName.Append((char)name_40); else break;
                    if (name_41 > 0) effectName.Append((char)name_41); else break;
                    if (name_42 > 0) effectName.Append((char)name_42); else break;
                    if (name_43 > 0) effectName.Append((char)name_43); else break;
                    if (name_44 > 0) effectName.Append((char)name_44); else break;
                    if (name_45 > 0) effectName.Append((char)name_45); else break;
                    if (name_46 > 0) effectName.Append((char)name_46); else break;
                    if (name_47 > 0) effectName.Append((char)name_47); else break;
                    if (name_48 > 0) effectName.Append((char)name_48); else break;
                    if (name_49 > 0) effectName.Append((char)name_49); else break;
                    if (name_50 > 0) effectName.Append((char)name_50); else break;
                    if (name_51 > 0) effectName.Append((char)name_51); else break;
                    if (name_52 > 0) effectName.Append((char)name_52); else break;
                    if (name_53 > 0) effectName.Append((char)name_53); else break;
                    if (name_54 > 0) effectName.Append((char)name_54); else break;
                    if (name_55 > 0) effectName.Append((char)name_55); else break;
                    if (name_56 > 0) effectName.Append((char)name_56); else break;
                    if (name_57 > 0) effectName.Append((char)name_57); else break;
                    if (name_58 > 0) effectName.Append((char)name_58); else break;
                    if (name_59 > 0) effectName.Append((char)name_59); else break;
                    if (name_60 > 0) effectName.Append((char)name_60); else break;
                    if (name_61 > 0) effectName.Append((char)name_61); else break;
                    if (name_62 > 0) effectName.Append((char)name_62); else break;
                    if (name_63 > 0) effectName.Append((char)name_63); else break;
                }

                return effectName.ToString();
            }
        }

        public Binary_effect[] m_Effects;

        public bool construct(Binary_effect[] effects)
        {
            m_Effects = effects;
            return true;
        }

        public void Dispose()
        {
        }
    }
}
