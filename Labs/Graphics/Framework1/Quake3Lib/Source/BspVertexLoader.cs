using System;
using System.Diagnostics;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using BlackRice.Framework.Quake3.Load;

namespace BlackRice.Framework.Quake3
{
    class BspVertexLoader
    {
        FieldInfo[] m_TypeFields;
        VertexElement[] m_SemanticEls;
        BspFile.CoordSysConv m_Conv;

        Vector3 vector3 = new Vector3();
        Vector2 vector2 = new Vector2();
        Color color = new Color();

        public BspVertexLoader(FieldInfo[] typeFields, VertexElement[] semanticEls, BspFile.CoordSysConv conv)
        {
            m_TypeFields = typeFields;
            m_SemanticEls = semanticEls;
            m_Conv = conv;
        }

        public void Read(BspFile.Vertices.Binary_vertex vertex, ref object outVertex)
        {
            for (int fieldIndex = 0; fieldIndex < m_SemanticEls.Length; ++fieldIndex)
            {
                VertexElement vEl = m_SemanticEls[fieldIndex];
                FieldInfo fieldInfo = m_TypeFields[fieldIndex];

                switch (vEl.VertexElementUsage)
                {
                    case VertexElementUsage.Position:
                    {
                        // Trace.Assert(vEl.VertexElementFormat == VertexElementFormat.Vector3);
                        // Trace.Assert(fieldInfo.FullName == "Microsoft.Xna.Framework.Vector3");

                        vector3.X = vertex.position_x;
                        vector3.Y = vertex.position_y;
                        vector3.Z = vertex.position_z;
                        m_Conv.ConvertVector(ref vector3);

                        fieldInfo.SetValue(outVertex, vector3);
                    }
                    break;

                    case VertexElementUsage.Color:
                    {
                        // Trace.Assert(vEl.VertexElementFormat == VertexElementFormat.Color);
                        // Trace.Assert(fieldInfo.FullName == "Microsoft.Xna.Framework.Color");
                        color.R = vertex.color_r;
                        color.G = vertex.color_g;
                        color.B = vertex.color_b;
                        color.A = vertex.color_a;

                        fieldInfo.SetValue(outVertex, color);
                    }
                    break;

                    case VertexElementUsage.TextureCoordinate:
                    {
                        // Trace.Assert(vEl.VertexElementFormat == VertexElementFormat.Vector2);
                        // Trace.Assert(fieldInfo.FullName == "Microsoft.Xna.Framework.Vector2");

                        if (vEl.UsageIndex == 0)
                        {
                            vector2.X = vertex.texcoord_s;
                            vector2.Y = vertex.texcoord_t;
                        }
                        else if (vEl.UsageIndex == 1)
                        {
                            vector2.X = vertex.texcoord_lm_s;
                            vector2.Y = vertex.texcoord_lm_t;
                        }
                        else
                        {
                            Trace.Assert(false, "Vertex Layout is incompatible");
                        }
                       
                        m_Conv.ConvertTexCoord(ref vector2);

                        fieldInfo.SetValue(outVertex, vector2);
                    }
                    break;

                    default:
                    {
                        Trace.Assert(false, "Vertex Layout is incompatible");
                    }
                    break;
                }
            }
        }
    }
}