using System;

namespace Framework1.Geometry
{
    public class UniformGridTesselator
    {
        public static int GetTriangleStripSize(Int16 gridSizeX, Int16 gridSizeY)
        {
            return ((gridSizeY - 1) * ((gridSizeX - 1) * 2)) + ((gridSizeY - 2) * 2);
        }

        public static void GenerateTriangleStrip(Int16 gridSizeX, Int16 gridSizeY, ref Int16[] indices, int indexOffset)
        {
            int i = indexOffset;

            for (Int16 y = 0, rowOffset = 0, nextRowOffset = 1; y < gridSizeY; ++y, rowOffset += gridSizeX, nextRowOffset += gridSizeX)
            {
                for (Int16 x = 0; x < gridSizeX; ++x)
                {
                    indices[i++] = (Int16)(rowOffset + x);
                    indices[i++] = (Int16)(nextRowOffset + x);
                }

                // stitch with degenerate tri
                indices[i++] = indices[i-1];
                indices[i++] = indices[nextRowOffset + gridSizeX + 0];
            }
        }
    }
}
