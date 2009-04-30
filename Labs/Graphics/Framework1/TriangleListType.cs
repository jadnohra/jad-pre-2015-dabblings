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
    }
}