using System;
using System.IO;
using System.Text;

namespace BlackRice.Framework.Quake3.Load
{
    public class ScriptBlockParser
    {
        public struct BlockAddress
        {
            public int startLine;
            public int lineCount;

            public BlockAddress(int startLine_, int lineCount_)
            {
                startLine = startLine_;
                lineCount = lineCount_;
            }
        }

        FileStream m_Stream;
        TextReader m_Reader;
        bool m_HasError = false;
        int m_ReadLineCount = 0;
        FileStream m_ReusableStream;
        TextReader m_ReusableReader;
        static char[] TrimChars = { ' ', '\n', '\r', '\t' };

        public ScriptBlockParser()
        {
        }

        public void Open(string filePath)
        {
            m_HasError = false;
            m_Stream = File.OpenRead(filePath);
            m_Reader = new StreamReader(m_Stream);
            m_ReadLineCount = 0;
            m_ReusableStream = File.OpenRead(filePath);
            m_ReusableReader = new StreamReader(m_ReusableStream);
        }

        public bool HasError() { return m_HasError; }

        static string Clean(string text)
        {
            return text == null ? null : text.Trim(TrimChars);
        }

        string ReadRawLine()
        {
            ++m_ReadLineCount;
            return Clean(m_Reader.ReadLine());
        }

        bool IsCommentLine(string line)
        {
            return line.StartsWith("//");
        }

        string ReadLine()
        {
            string line = ReadRawLine();

            while (line != null && IsCommentLine(line))
            {
                line = ReadRawLine();
            }

            return line;
        }

        public bool GetNextScriptAddress(ref string name, ref BlockAddress address)
        {
            name = null;

            if (m_HasError)
                return false;

            address.startLine = m_ReadLineCount;
            string line = ReadLine();

            while (line != null && 
                    (line.Length == 0))
            {
                address.startLine = (int)m_Stream.Position;
                line = ReadLine();
            }

            if (line == null)
                return false;

            name = line;

            line = m_Reader.ReadLine();
            while (line != null &&
                    (line.Length == 0))
            {
                line = ReadLine();
            }
           
            if (line != "{")
            {
                m_HasError = true;
                return false;
            }

            int subCount = 0;
            while (line != null &&
                    (line.Length == 0 || subCount >= 0))
            {
                line = ReadLine();

                if (line == null)
                {
                    m_HasError = true;
                    return false;
                }
                else if (line == "{")
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

            address.lineCount = (m_ReadLineCount - address.startLine);

            return true;
        }

        public string GetScriptSource(BlockAddress address)
        {
            m_ReusableStream.Seek(0, SeekOrigin.Begin);

            for (int i = 0; i < address.startLine; ++i)
                m_ReusableReader.ReadLine();

            StringBuilder builder = new StringBuilder();

            for (int i = 0; i < address.lineCount; ++i)
            {
                builder.AppendLine(ScriptBlockParser.Clean(m_ReusableReader.ReadLine()));
            }

            return builder.ToString();
        }
    }
}