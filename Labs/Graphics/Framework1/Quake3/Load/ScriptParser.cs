using System;
using System.IO;

namespace Framework1.Quake3.Load
{

    public class ScriptBlockParser
    {
        FileStream m_Stream;
        TextReader m_Reader;
        bool m_HasError = false;

        public ScriptBlockParser()
        {
        }

        public void Open(string filePath)
        {
            m_HasError = false;
            m_Stream = File.OpenRead(filePath);
            m_Reader = new StreamReader(m_Stream);
        }

        public bool HasError() { return m_HasError; }

        public bool GetNextScriptAddress(ref string name, ref int pos, ref int count)
        {
            // Trim does not work for \t
            // positions are wrong becuase of buffering
            // fix those 2 issues.
            name = null;

            if (m_HasError)
                return false;
            
            pos = (int) m_Stream.Position;
            string line = m_Reader.ReadLine();

            while (line != null && 
                    (line.Length == 0))
            {
                pos = (int) m_Stream.Position;
                line = m_Reader.ReadLine();
            }

            if (line == null)
                return false;

            name = line;

            line = m_Reader.ReadLine();
            while (line != null &&
                    (line.Length == 0))
            {
                line = m_Reader.ReadLine();
            }
            line.Trim();

            if (line != "{")
            {
                m_HasError = true;
                return false;
            }

            int subCount = 0;
            while (line != null &&
                    (line.Length == 0 || subCount >= 0))
            {
                line = m_Reader.ReadLine();
                line.Trim();

                if (line == "{")
                {
                    ++subCount;
                }
                else if (line == "}")
                {
                    --subCount;
                }
            }

            if (line == null || subCount != -1)
            {
                m_HasError = true;
                return false;
            }

            count = (int) (m_Stream.Position - pos);

            return true;
        }
    }

    public class ScriptBlockLoader
    {
        FileStream m_Stream;
        TextReader m_Reader;
        char[] m_Buffer;

        public ScriptBlockLoader(string filePath)
        {
            m_Stream = File.OpenRead(filePath);
            m_Reader = new StreamReader(m_Stream);
        }

        public string GetScriptSource(int pos, int count)
        {
            m_Stream.Position = pos;
            
            if (m_Buffer == null || m_Buffer.Length < count)
                m_Buffer = new char[count];

            m_Reader.ReadBlock(m_Buffer, pos, count);

            return new string(m_Buffer);
        }
    }
}