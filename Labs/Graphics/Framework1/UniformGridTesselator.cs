using System;
using System.Diagnostics;

namespace Framework1.Geometry
{
    public class UniformGridTesselator
    {
        public static int GetTriangleStripSize(int gridSizeX, int gridSizeY)
        {
            int degenerateIndexCount = (gridSizeY - 2) * 4;
            int rowCount = (gridSizeY - 1);
            int indexCountPerRow = 2 + ((gridSizeX - 1) * 2);
            return (rowCount * indexCountPerRow) + degenerateIndexCount;
        }

        public static void GenerateTriangleStrip<T>(int gridSizeX, int gridSizeY, ref T[] indices, int indexOffset, bool clockwizeWind)
        {
            GenerateTriangleStrip<T>(gridSizeX, gridSizeY, ref indices, indexOffset, 0, gridSizeX, clockwizeWind);
        }

        public static void GenerateTriangleStrip<T>(int gridSizeX, int gridSizeY, ref T[] indices, int indexOffset, int startIndex, int stride, bool clockwizeWind)
        {
            int i = indexOffset;
            
            if (clockwizeWind)
            {
                for (int y = 0, rowOffset = startIndex, nextRowOffset = startIndex + stride; y < gridSizeY - 1; ++y, rowOffset += stride, nextRowOffset += stride)
                {
                    for (int x = 0; x < gridSizeX; ++x)
                    {
                        indices[i++] = (T)Convert.ChangeType(rowOffset + x, typeof(T));
                        indices[i++] = (T)Convert.ChangeType(nextRowOffset + x, typeof(T));
                    }

                    if (y + 1 < gridSizeY - 1)
                    {
                        // stitch with degenerate tris
                        indices[i] = indices[i - 1];
                        ++i;
                        indices[i] = (T)Convert.ChangeType(nextRowOffset + stride + 0, typeof(T));
                        ++i;
                        indices[i] = indices[i - 1];
                        ++i;
                        // This one is to keep the winding consistent
                        indices[i] = indices[i - 1];
                        ++i;
                    }
                }
            }
            else
            {
                for (int x = 0; x < gridSizeX - 1; ++x)
                {
                    for (int y = 0, rowOffset = startIndex + x; y < gridSizeY; ++y, rowOffset += stride)
                    {
                        indices[i++] = (T)Convert.ChangeType(rowOffset + 0, typeof(T));
                        indices[i++] = (T)Convert.ChangeType(rowOffset + 1, typeof(T));
                    }

                    if (x + 1 < gridSizeX - 1)
                    {
                        // stitch with degenerate tris
                        indices[i] = indices[i - 1];
                        ++i;
                        indices[i] = (T)Convert.ChangeType(startIndex + x, typeof(T));
                        ++i;
                        indices[i] = indices[i - 1];
                        ++i;
                        // This one is to keep the winding consistent
                        indices[i] = indices[i - 1];
                        ++i;
                    }
                }
            }
            
            Trace.Assert(GetTriangleStripSize(gridSizeX, gridSizeY) == i - indexOffset);
        }

        public static int GetTriangleStripSize(int gridCountX, int gridCountY, int gridSizeX, int gridSizeY)
        {
            int gridTriStripSize = GetTriangleStripSize(gridSizeX, gridSizeY);

            int degenerateIndexCount = (gridCountY * (gridCountX - 1) * 4) + ((gridCountY - 1) * 4);
            return (gridCountX * gridCountY * gridTriStripSize) + degenerateIndexCount;
        }

        public static void GenerateTriangleStrip<T>(int gridCountX, int gridCountY, int gridSizeX, int gridSizeY, ref T[] indices, int indexOffset, bool clockwizeWind)
        {
            int gridTriStripSize = GetTriangleStripSize(gridSizeX, gridSizeY);
            int stride = gridSizeX * gridCountX;
            int stitchSourceIndex = -1;
            int i = indexOffset;

            for (int gridY = 0; gridY < gridCountY; ++gridY)
            {
                for (int gridX = 0; gridX < gridCountX; ++gridX)
                {
                    int startIndex = (gridY * stride) + (gridX * gridSizeX);

                    if (stitchSourceIndex >= 0)
                    {
                        indices[i] = indices[i - 1];
                        ++i;
                        indices[i] = (T)Convert.ChangeType(startIndex + indexOffset + 0, typeof(T));
                        ++i;
                        indices[i] = indices[i - 1];
                        ++i;
                        // This one is to keep the winding consistent
                        indices[i] = indices[i - 1];
                        ++i;
                    }

                    GenerateTriangleStrip<T>(gridSizeX, gridSizeY, ref indices, indexOffset + i, startIndex, stride, clockwizeWind);
                    i += gridTriStripSize;
                    stitchSourceIndex = startIndex;
                }
            }

            Trace.Assert(GetTriangleStripSize(gridCountX, gridCountY, gridSizeX, gridSizeY) == i - indexOffset);
        }
    }
}
