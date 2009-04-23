using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Framework1
{
    class StreamName
    {
        StreamNames names;
        string name;

        public StreamName(StreamNames parent, string str)
        {
            names = parent;
            name = str;
        }

        public static bool operator ==(StreamName name1, StreamName name2)
        {
            return name1.names == name2.names
                 ? Object.ReferenceEquals(name1.name, name2.name)
                 : name1.name == name2.name;
        }

        public static bool operator !=(StreamName name1, StreamName name2)
        {
            return name1.names == name2.names
                 ? !Object.ReferenceEquals(name1.name, name2.name)
                 : name1.name != name2.name;
        }

        public override int GetHashCode()
        {
            return name.GetHashCode();
        }

        public override string ToString()
        {
            return name;
        }
    }

    class StreamNames
    {
        public static StreamName tri_indices;

        StreamNames()
        {
            tri_indices = new StreamName(this, "tri_indices");
        }
    }
}
