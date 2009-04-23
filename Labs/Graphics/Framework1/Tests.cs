﻿using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;
using System.Reflection;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Storage;
using Framework1.Quake3;

namespace Framework1
{
    using Quake3.BspFile;

    class UnitTests : IDisposable
    {
        public void Dispose()
        {
            GC.Collect();
        }

        public void Main(string[] args) 
        {
            Test_Types();
            Test_Quake3Loader();
        }

        [StructLayout(LayoutKind.Sequential)]
        public class FlexibleFormat1
        {
            public int i0;
            public int i1;
            public int i2;
            public float f0;
            public Vector3 v0; 	   
        }

         void Test_Types()
        {
            FlexibleFormat1 element1 = new FlexibleFormat1();
            Type type1 = typeof(FlexibleFormat1);

            Trace.Assert(type1.IsLayoutSequential == true);

            {
                FieldInfo info = type1.GetField("n/a");
                Trace.Assert(info == null);
            }

            {
                FieldInfo info = type1.GetField("i0");
                Trace.Assert(info != null);
                Trace.Assert(info.FieldType.IsValueType == true);
                Trace.Assert(info.FieldType.FullName == "System.Int32");

                info.SetValue(element1, 66);
                Trace.Assert(element1.i0 == 66);
            }

            {
                FieldInfo info = type1.GetField("f0");
                Trace.Assert(info != null);
                Trace.Assert(info.FieldType.IsValueType == true);
                Trace.Assert(info.FieldType.FullName == "System.Single");

                info.SetValue(element1, -66.0f);
                Trace.Assert(element1.f0 == -66.0f);
            }

            {
                FieldInfo info = type1.GetField("v0");
                Trace.Assert(info != null);
                Trace.Assert(info.FieldType.IsValueType == true);
                Trace.Assert(info.FieldType.FullName == "Microsoft.Xna.Framework.Vector3");

                info.SetValue(element1, new Vector3(-66.0f, 0.0f, 66.0f));
                Trace.Assert(element1.v0.X == -66.0f);
            }

            {
                VertexPositionColor vertex = new VertexPositionColor();
                Type type2 = typeof(VertexPositionColor);

                FieldInfo info = type2.GetField("Position");
                Trace.Assert(info != null);
                Trace.Assert(info.FieldType.IsValueType == true);
                Trace.Assert(info.FieldType.FullName == "Microsoft.Xna.Framework.Vector3");

                object p = (object) vertex; 
                info.SetValue(p, new Vector3(-66.0f, 0.0f, 66.0f));
                vertex = (VertexPositionColor)p; // boxing is needed!
                Trace.Assert(vertex.Position.X == -66.0f); 
             }
        }

        void Test_Quake3Loader()
        {
            /*
            string filePath = Path.Combine(StorageContainer.TitleLocation, "test.bsp");
            
            BspLoader loader = new BspLoader(filePath);
            Header header = loader.Open();

            Trace.Assert(header != null);

            int faceCount = loader.GetFaceCount(header);

            if (faceCount > 0)
            {
                using (Faces faces = loader.GetFaces(header, 0, faceCount))
                {
                    Trace.Assert(faces != null);
                }
            }

            int vertexCount = loader.GetVertexCount(header);

            if (vertexCount > 0)
            {
                using (Vertices vertices = loader.GetVertices(header, 0, vertexCount))
                {
                    Trace.Assert(vertices != null);
                }
            }
             */ 
        }
    }
}