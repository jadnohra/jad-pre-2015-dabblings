using System;
using System.Collections.Generic;
using System.Diagnostics;
using Microsoft.Xna.Framework;

namespace Framework1.Quake3
{
    public class BspTree
    {
        public void Evict(RenderResourceBlockCollector collector)
        {
            //Note: this is a low frequency low count process, no need to 
            // check whole tree every frame! find a more modular solution for this
            /*
            foreach (Leaf leaf in m_Leafs)
            {
                if (collector.EvictIfNeeded(leaf.renderResourceBlock))
                {
                    leaf.renderResourceBlock = null;
                }
            }
             */ 
        }

        public void FindVisibleLeafs(Vector3 point, VisibleLeafs visibleLeafs, bool filterOutEmptyFaceLeafs)
        {
            int pointLeafIndex = FindLeaf(point);
            visibleLeafs.Clear();

            if (pointLeafIndex >= 0)
            {
                Leaf pointLeaf = m_Leafs[pointLeafIndex];
                int pointLeafCluster = pointLeaf.cluster;
                
                foreach (Leaf leaf in m_Leafs)
                {
                    if (IsClusterVisible(pointLeafCluster, leaf.cluster))
                    {
                        if (leaf.leafFaceCount > 0 || !filterOutEmptyFaceLeafs)
                        {
                            visibleLeafs.Add(leaf);
                        }
                    }
                }
            }
        }

        public int FindLeaf(Vector3 point) 
        {
            int index = 0;

            while (index >= 0) {

                Node  node  = m_Nodes[index];
                Plane plane = m_Planes[node.plane];

                // Distance from point to a plane
                float distance = Vector3.Dot(plane.normal, point) - plane.dist;

                if (distance >= 0) {
                    index = node.child[(int)PlaneSide.Front];
                } else {
                    index = node.child[(int)PlaneSide.Back];
                }
            }

            return -index - 1;
        }

        public bool IsClusterVisible(int visCluster, int testCluster) 
        {
            //return true;
            // testCluster can be -1, this is not a problem.

            if (m_Visdata == null || visCluster < 0) 
            {
                return true;
            }

            int i = (visCluster * m_Visdata.m_Info.sz_vecs) + (testCluster >> 3);
            byte visSet = m_Visdata.m_Data[i].data;
            return (visSet & (1 << (testCluster & 7))) != 0;
        }

        public struct LeafFaceInterval
        {
            public int firstFaceIndex;
            public int faceCount;
        }

        public bool Load(LoadedBspLevel loadedLevel)
        {
            m_Level = loadedLevel;
            BspFile.Header header = loadedLevel.Header;
            BspFile.CoordSysConv conv = loadedLevel.CoordSysConv;

            using (BspFile.Planes planes = header.Loader.GetPlanes(header, 0, header.Loader.GetPlaneCount(header)))
            {
                m_Planes = new Plane[planes.m_Planes.Length];

                for (int i = 0; i < planes.m_Planes.Length; ++i)
                {
                    m_Planes[i] = new Plane();
                    Convert(conv, planes.m_Planes[i], ref m_Planes[i]);
                }
            }

            using (BspFile.Nodes nodes = header.Loader.GetNodes(header, 0, header.Loader.GetNodeCount(header)))
            {
                m_Nodes = new Node[nodes.m_Nodes.Length];

                for (int i = 0; i < nodes.m_Nodes.Length; ++i)
                {
                    m_Nodes[i] = new Node();
                    Convert(conv, nodes.m_Nodes[i], ref m_Nodes[i]);
                }
            }

            using (BspFile.LeafFaces leafFaces = header.Loader.GetLeafFaces(header, 0, header.Loader.GetLeafFaceCount(header)))
            {
                m_LeafFaces = new LeafFace[leafFaces.m_LeafFaces.Length];

                // Optimization: we could filter out leafs with faces that are not renderable (Polygon or Mesh type)
                for (int i = 0; i < leafFaces.m_LeafFaces.Length; ++i)
                {
                    m_LeafFaces[i] = new LeafFace();
                    Convert(conv, leafFaces.m_LeafFaces[i], ref m_LeafFaces[i]);
                }
            }

            using (BspFile.Leafs leafs = header.Loader.GetLeafs(header, 0, header.Loader.GetLeafCount(header)))
            {
                m_Leafs = new Leaf[leafs.m_Leafs.Length];

                // Optimization: we could filter out leafs with faces that are not renderable (Polygon or Mesh type)
                for (int i = 0; i < leafs.m_Leafs.Length; ++i)
                {
                    m_Leafs[i] = new Leaf();
                    Convert(conv, leafs.m_Leafs[i], ref m_Leafs[i]);
                }
            }

            m_Visdata = header.Loader.GetVisdata(header);

            return true;
        }

        void Convert(BspFile.CoordSysConv conv, BspFile.Nodes.Binary_node from, ref Node to)
        {
            to.child = new int[2];
           
            to.plane = from.plane;
            to.child[(int)PlaneSide.Front] = from.child_0_front;
            to.child[(int)PlaneSide.Back] = from.child_1_back;
            // Note: Do not forget to convert min and max if you use them.
        }

        void Convert(BspFile.CoordSysConv conv, BspFile.Leafs.Binary_leaf from, ref Leaf to)
        {
            to.cluster = from.cluster;
            to.firstLeafFace = from.leafface;
            to.leafFaceCount = from.n_leaffaces;
            // Note: Do not forget to convert min and max if you use them.
        }

        void Convert(BspFile.CoordSysConv conv, BspFile.LeafFaces.Binary_leafface from, ref LeafFace to)
        {
            to.faceIndex = from.face;
        }

        void Convert(BspFile.CoordSysConv conv, BspFile.Planes.Binary_plane from, ref Plane to)
        {
            to.normal.X = from.normal_0;
            to.normal.Y = from.normal_1;
            to.normal.Z = from.normal_2;

            conv.ConvertVector(ref to.normal);

            to.dist = from.dist;
        }

        enum PlaneSide
        {
            Front = 0, Back = 1, 
        }

        struct Node
        {
            public int plane; 	    //Plane index.
            public int[] child; 	// Children indices. Negative numbers are leaf indices: -(leaf+1).
        }

        public class Leaf
        {
            public int cluster; 	        // visdata cluster index.
            public int firstLeafFace; 	    // First leafface for leaf.
            public int leafFaceCount; 	    // Number of leaffaces for leaf.

            public LeafRenderResourceBlock renderResourceBlock;
        }

        public class LeafFace
        {
            public int faceIndex;
        }


        struct Plane
        {
            public Vector3 normal;
            public float dist;
        }

        public class VisibleLeafs : List<Leaf>
        {
        }

        public LoadedBspLevel m_Level;
        Plane[] m_Planes;
        Node[] m_Nodes;
        Leaf[] m_Leafs;
        public LeafFace[] m_LeafFaces;
        BspFile.Visdata m_Visdata;
    }
}
