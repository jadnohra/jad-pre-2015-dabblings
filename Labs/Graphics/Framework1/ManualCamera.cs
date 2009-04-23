using Microsoft.Xna.Framework;
using Microsoft.Xna.Framework.Graphics;
using Microsoft.Xna.Framework.Input;

namespace Framework1
{
    public class ManualCamera
    {
        public ManualCamera()
        {
            worldTransform = Matrix.Identity;

            isRotating = false;
            lastScrollValue = 0;

            moveScale = 1.0f;

            zFar = 100.0f;
            zNear = 0.1f;
        }

        public void SetNearFarPlanes(float zn, float zf)
        {
            zFar = zf;
            zNear = zn;
        }

        public void SetMoveScale(float scale)
        {
            moveScale = scale;
        }

        public void SetSceneSize(float size)
        {
            moveScale = size;
            zFar = size * 100.0f;
            zNear = size / 10000.0f;
        }

        public float GetZFar()
        {
            return zFar;
        }

        public float GetZNear()
        {
            return zNear;
        }
        
        public void SetViewMatrix(Matrix matrix)
        {
            viewMatrix = matrix;

            worldTransform = Matrix.Invert(viewMatrix);
        }

        public Matrix GetViewMatrix()
        {
            return viewMatrix;
        }

        public Matrix GetWorldTransform()
        {
            return worldTransform;
        }

        public void Move(Vector3 distance)
        {
            moveDistance += distance;
        }

        public void Rotate(Vector3 axis, float angle, int index)
        {
            rotQuaternion[index] *= Quaternion.CreateFromAxisAngle(axis, angle);
        }

        public void StartRotate(Vector3 screenPoint)
        {
            if (!isRotating)
            {
                isRotating = true;
                rotStartPoint = screenPoint;
                rotBaseWorldDirTransform = worldTransform;
                rotBaseWorldDirTransform.Translation = Vector3.Zero;
             }
        }

        public void StopRotate()
        {
            isRotating = false;
        }

        public void Update(Viewport viewport, Matrix projection, Vector3 screenPoint)
        {
            if (isRotating)
            {
                Vector3 worldFromPos = viewport.Unproject(rotStartPoint, projection, viewMatrix, Matrix.Identity);
                Vector3 worldToPos = viewport.Unproject(screenPoint, projection, viewMatrix, Matrix.Identity);

                Vector3 fromDir = worldFromPos - worldTransform.Translation;
                Vector3 toDir = worldToPos - worldTransform.Translation;

                if (Mouse.GetState().RightButton == ButtonState.Pressed)
                {
                    fromDir.Y = toDir.Y;
                }

                fromDir.Normalize();
                toDir.Normalize();

                Vector3 rotAxis = Vector3.Cross(fromDir, toDir);
                Quaternion quat;

                float rotAngle = Vector3.Dot(fromDir, toDir) * 1.5f;
                quat = Quaternion.CreateFromAxisAngle(rotAxis, rotAngle);

                quat.Normalize();
                

                if (rotAxis != Vector3.Zero) 
                {
                    Matrix rotMatrix = Matrix.CreateFromQuaternion(quat);
                    Vector3 trans = worldTransform.Translation;
                    worldTransform = Matrix.Multiply(rotBaseWorldDirTransform, rotMatrix);
                    worldTransform.Translation = trans;
                }
            }

            if (moveDistance != Vector3.Zero)
            {
                Matrix dirMatrix = worldTransform;
                dirMatrix.Translation = Vector3.Zero;

                Vector3 trans = Vector3.Transform(moveDistance, dirMatrix);

                worldTransform.Translation += trans;

                moveDistance = Vector3.Zero;
            }

            {
                Vector3 translation = worldTransform.Translation;
                worldTransform.Translation = Vector3.Zero;

                {
                    worldTransform *= Matrix.CreateFromQuaternion(rotQuaternion[0]);
                    worldTransform *= Matrix.CreateFromQuaternion(rotQuaternion[1]);
                }

                worldTransform.Translation = translation;

                rotQuaternion[0] = Quaternion.Identity;
                rotQuaternion[1] = Quaternion.Identity;
            }

            viewMatrix = Matrix.Invert(worldTransform);
        }

        public void UpdateInput(Viewport viewport, Matrix projection, GameTime gameTime)
        {
            float distance = ((float)gameTime.ElapsedGameTime.TotalSeconds * moveScale);

            {
                KeyboardState keybState = Keyboard.GetState();

                if (keybState.IsKeyDown(Keys.LeftShift))
                    distance *= 10.0f;
                if (keybState.IsKeyDown(Keys.LeftControl))
                    distance *= 0.1f;

                if (keybState.IsKeyDown(Keys.W))
                    this.Move(Vector3.Forward * distance);
                else if (keybState.IsKeyDown(Keys.S))
                    this.Move(Vector3.Backward * distance);

                if (keybState.IsKeyDown(Keys.D))
                    this.Move(Vector3.Right * distance);
                else if (keybState.IsKeyDown(Keys.A))
                    this.Move(Vector3.Left * distance);

                if (keybState.IsKeyDown(Keys.Left))
                    Rotate(Vector3.Up, MathHelper.PiOver2 * 0.01f, 0);
                else if (keybState.IsKeyDown(Keys.Right))
                    Rotate(Vector3.Up, -MathHelper.PiOver2 * 0.01f, 0);

                if (keybState.IsKeyDown(Keys.Up))
                    Rotate(worldTransform.Right, MathHelper.PiOver2 * 0.01f, 1);
                else if (keybState.IsKeyDown(Keys.Down))
                    Rotate(worldTransform.Right, -MathHelper.PiOver2 * 0.01f, 1);

            }

            Vector3 mousePoint = Vector3.Zero;

            {
                MouseState mouseState = Mouse.GetState();

                if (mouseState.ScrollWheelValue != lastScrollValue)
                {
                    this.Move(Vector3.Forward * distance * (float)(mouseState.ScrollWheelValue - lastScrollValue));
                    lastScrollValue = mouseState.ScrollWheelValue;
                }
                
                if (mouseState.MiddleButton == ButtonState.Pressed)
                {
                    mousePoint = new Vector3(mouseState.X, mouseState.Y, 0.0f);
                    this.StartRotate(mousePoint);
                }
                else
                {
                    this.StopRotate();
                }
            }

            this.Update(viewport, projection, mousePoint);
        }

        float zFar;
        float zNear;

        float moveScale;

        Matrix worldTransform;
        Matrix viewMatrix;

        int lastScrollValue;

        Vector3 moveDistance;
        Quaternion[] rotQuaternion = { Quaternion.Identity, Quaternion.Identity };

        bool isRotating;
        Vector3 rotStartPoint;
        Matrix rotBaseWorldDirTransform;
    }
}
