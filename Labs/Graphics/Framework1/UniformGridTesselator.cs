using System;
using System.Diagnostics;

namespace Framework1.Geometry
{
    public class UniformGridTesselator
    {
        public static int GetTriangleStripSize(int gridSizeX, int gridSizeY)
        {
            int degenerateIndexCount = (gridSizeY - 2) * 3;
            int rowCount = (gridSizeY - 1);
            int indexCountPerRow = 2 + ((gridSizeX - 1) * 2);
            return (rowCount * indexCountPerRow) + degenerateIndexCount;
        }

        public static void GenerateTriangleStrip<T>(int gridSizeX, int gridSizeY, ref T[] indices, int indexOffset)
        {
            int i = indexOffset;

            for (int y = 0, rowOffset = 0, nextRowOffset = gridSizeX; y < gridSizeY - 1; ++y, rowOffset += gridSizeX, nextRowOffset += gridSizeX)
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
                    indices[i] = (T)Convert.ChangeType(nextRowOffset + gridSizeX + 0, typeof(T));
                    ++i;
                    indices[i] = indices[i - 1];
                    ++i;
                }
            }

            Trace.Assert(GetTriangleStripSize(gridSizeX, gridSizeY) == i - indexOffset);
        }
    }
}
