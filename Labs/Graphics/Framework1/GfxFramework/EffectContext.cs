using System;
using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;

namespace Framework1
{
    public class EffectContext
    {
        Matrix m_ViewMatrix;
        Matrix m_ProjectionMatrix;

        bool m_IsDirtyViewProjectionMatrix;
        Matrix m_ViewProjectionMatrix;

        public Matrix GetViewMatrix()
        {
            return m_ViewMatrix;
        }

        public Matrix GetProjectionMatrix()
        {
            return m_ProjectionMatrix;
        }

        public Matrix GetViewProjectionMatrix()
        {
            if (m_IsDirtyViewProjectionMatrix)
            {
                m_ViewProjectionMatrix = Matrix.Multiply(m_ViewMatrix, m_ProjectionMatrix);
            }

            return m_ViewProjectionMatrix;
        }

        public void SetViewMatrix(Matrix matrix)
        {
            m_ViewMatrix = matrix;
            m_IsDirtyViewProjectionMatrix = true;
        }

        public void SetProjectionMatrix(Matrix matrix)
        {
            m_ProjectionMatrix = matrix;
            m_IsDirtyViewProjectionMatrix = true;
        }
    }
}