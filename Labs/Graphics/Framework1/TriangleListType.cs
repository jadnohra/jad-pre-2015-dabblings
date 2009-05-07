using System;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1
{
    public class TriangleListTypes
    {
        public static int ToPrimitiveCount(PrimitiveType type, int indexCount)
        {
            if (type == PrimitiveType.TriangleList)
                return indexCount / 3;

            return indexCount - 2;
        }

        public static void InvertTriangleWinding<T>(PrimitiveType type,
                                                    T[] sourceIndices, int startIndex, int count,
                                                    ref T[] targetIndices, int targetStartIndex)
        {
            if (type == PrimitiveType.TriangleStrip)
            {
                for (int i = 0, source = startIndex, target = targetStartIndex + (count - 1);
                     i < count; ++i, ++source, --target)
                {
                    targetIndices[target] = sourceIndices[source];
                }
            }
            else
            {
                throw new Exception("Not Implemented");
            }
        }
    }
}