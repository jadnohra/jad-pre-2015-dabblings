
using System;
using System.IO;
using System.Collections;

namespace BlackRice.Framework.Quake3.ShaderParser {

public class Token {
	public int kind;    // token kind
	public int pos;     // token position in the source text (starting at 0)
	public int col;     // token column (starting at 1)
	public int line;    // token line (starting at 1)
	public string val;  // token value
	public Token next;  // ML 2005-03-11 Tokens are kept in linked list
}

//-----------------------------------------------------------------------------------
// Buffer
//-----------------------------------------------------------------------------------
public class Buffer {
	// This Buffer supports the following cases:
	// 1) seekable stream (file)
	//    a) whole stream in buffer
	//    b) part of stream in buffer
	// 2) non seekable stream (network, console)

	public const int EOF = char.MaxValue + 1;
	const int MIN_BUFFER_LENGTH = 1024; // 1KB
	const int MAX_BUFFER_LENGTH = MIN_BUFFER_LENGTH * 64; // 64KB
	byte[] buf;         // input buffer
	int bufStart;       // position of first byte in buffer relative to input stream
	int bufLen;         // length of buffer
	int fileLen;        // length of input stream (may change if the stream is no file)
	int bufPos;         // current position in buffer
	Stream stream;      // input stream (seekable)
	bool isUserStream;  // was the stream opened by the user?
	
	public Buffer (Stream s, bool isUserStream) {
		stream = s; this.isUserStream = isUserStream;
		
		if (stream.CanSeek) {
			fileLen = (int) stream.Length;
			bufLen = Math.Min(fileLen, MAX_BUFFER_LENGTH);
			bufStart = Int32.MaxValue; // nothing in the buffer so far
		} else {
			fileLen = bufLen = bufStart = 0;
		}

		buf = new byte[(bufLen>0) ? bufLen : MIN_BUFFER_LENGTH];
		if (fileLen > 0) Pos = 0; // setup buffer to position 0 (start)
		else bufPos = 0; // index 0 is already after the file, thus Pos = 0 is invalid
		if (bufLen == fileLen && stream.CanSeek) Close();
	}
	
	protected Buffer(Buffer b) { // called in UTF8Buffer constructor
		buf = b.buf;
		bufStart = b.bufStart;
		bufLen = b.bufLen;
		fileLen = b.fileLen;
		bufPos = b.bufPos;
		stream = b.stream;
		// keep destructor from closing the stream
		b.stream = null;
		isUserStream = b.isUserStream;
	}

	~Buffer() { Close(); }
	
	protected void Close() {
		if (!isUserStream && stream != null) {
			stream.Close();
			stream = null;
		}
	}
	
	public virtual int Read () {
		if (bufPos < bufLen) {
			return buf[bufPos++];
		} else if (Pos < fileLen) {
			Pos = Pos; // shift buffer start to Pos
			return buf[bufPos++];
		} else if (stream != null && !stream.CanSeek && ReadNextStreamChunk() > 0) {
			return buf[bufPos++];
		} else {
			return EOF;
		}
	}

	public int Peek () {
		int curPos = Pos;
		int ch = Read();
		Pos = curPos;
		return ch;
	}
	
	public string GetString (int beg, int end) {
		int len = end - beg;
		char[] buf = new char[len];
		int oldPos = Pos;
		Pos = beg;
		for (int i = 0; i < len; i++) buf[i] = (char) Read();
		Pos = oldPos;
		return new String(buf);
	}

	public int Pos {
		get { return bufPos + bufStart; }
		set {
			if (value >= fileLen && stream != null && !stream.CanSeek) {
				// Wanted position is after buffer and the stream
				// is not seek-able e.g. network or console,
				// thus we have to read the stream manually till
				// the wanted position is in sight.
				while (value >= fileLen && ReadNextStreamChunk() > 0);
			}

			if (value < 0 || value > fileLen) {
				throw new FatalError("buffer out of bounds access, position: " + value);
			}

			if (value >= bufStart && value < bufStart + bufLen) { // already in buffer
				bufPos = value - bufStart;
			} else if (stream != null) { // must be swapped in
				stream.Seek(value, SeekOrigin.Begin);
				bufLen = stream.Read(buf, 0, buf.Length);
				bufStart = value; bufPos = 0;
			} else {
				// set the position to the end of the file, Pos will return fileLen.
				bufPos = fileLen - bufStart;
			}
		}
	}
	
	// Read the next chunk of bytes from the stream, increases the buffer
	// if needed and updates the fields fileLen and bufLen.
	// Returns the number of bytes read.
	private int ReadNextStreamChunk() {
		int free = buf.Length - bufLen;
		if (free == 0) {
			// in the case of a growing input stream
			// we can neither seek in the stream, nor can we
			// foresee the maximum length, thus we must adapt
			// the buffer size on demand.
			byte[] newBuf = new byte[bufLen * 2];
			Array.Copy(buf, newBuf, bufLen);
			buf = newBuf;
			free = bufLen;
		}
		int read = stream.Read(buf, bufLen, free);
		if (read > 0) {
			fileLen = bufLen = (bufLen + read);
			return read;
		}
		// end of stream reached
		return 0;
	}
}

//-----------------------------------------------------------------------------------
// UTF8Buffer
//-----------------------------------------------------------------------------------
public class UTF8Buffer: Buffer {
	public UTF8Buffer(Buffer b): base(b) {}

	public override int Read() {
		int ch;
		do {
			ch = base.Read();
			// until we find a utf8 start (0xxxxxxx or 11xxxxxx)
		} while ((ch >= 128) && ((ch & 0xC0) != 0xC0) && (ch != EOF));
		if (ch < 128 || ch == EOF) {
			// nothing to do, first 127 chars are the same in ascii and utf8
			// 0xxxxxxx or end of file character
		} else if ((ch & 0xF0) == 0xF0) {
			// 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			int c1 = ch & 0x07; ch = base.Read();
			int c2 = ch & 0x3F; ch = base.Read();
			int c3 = ch & 0x3F; ch = base.Read();
			int c4 = ch & 0x3F;
			ch = (((((c1 << 6) | c2) << 6) | c3) << 6) | c4;
		} else if ((ch & 0xE0) == 0xE0) {
			// 1110xxxx 10xxxxxx 10xxxxxx
			int c1 = ch & 0x0F; ch = base.Read();
			int c2 = ch & 0x3F; ch = base.Read();
			int c3 = ch & 0x3F;
			ch = (((c1 << 6) | c2) << 6) | c3;
		} else if ((ch & 0xC0) == 0xC0) {
			// 110xxxxx 10xxxxxx
			int c1 = ch & 0x1F; ch = base.Read();
			int c2 = ch & 0x3F;
			ch = (c1 << 6) | c2;
		}
		return ch;
	}
}

//-----------------------------------------------------------------------------------
// Scanner
//-----------------------------------------------------------------------------------
public class Scanner {
	const char EOL = '\n';
	const int eofSym = 0; /* pdt */
	const int maxT = 141;
	const int noSym = 141;


	public Buffer buffer; // scanner buffer
	
	Token t;          // current token
	int ch;           // current input character
	int pos;          // byte position of current character
	int col;          // column number of current character
	int line;         // line number of current character
	int oldEols;      // EOLs that appeared in a comment;
	static readonly Hashtable start; // maps first token character to start state

	Token tokens;     // list of tokens already peeked (first token is a dummy)
	Token pt;         // current peek token
	
	char[] tval = new char[128]; // text of current token
	int tlen;         // length of current token
	
	static Scanner() {
		start = new Hashtable(128);
		for (int i = 65; i <= 90; ++i) start[i] = 1;
		for (int i = 97; i <= 122; ++i) start[i] = 1;
		for (int i = 48; i <= 57; ++i) start[i] = 2;
		start[45] = 32; 
		start[40] = 4; 
		start[41] = 5; 
		start[123] = 6; 
		start[36] = 33; 
		start[42] = 25; 
		start[125] = 31; 
		start[Buffer.EOF] = -1;

	}
	
	public Scanner (string fileName) {
		try {
			Stream stream = new FileStream(fileName, FileMode.Open, FileAccess.Read, FileShare.Read);
			buffer = new Buffer(stream, false);
			Init();
		} catch (IOException) {
			throw new FatalError("Cannot open file " + fileName);
		}
	}
	
	public Scanner (Stream s) {
		buffer = new Buffer(s, true);
		Init();
	}
	
	void Init() {
		pos = -1; line = 1; col = 0;
		oldEols = 0;
		NextCh();
		if (ch == 0xEF) { // check optional byte order mark for UTF-8
			NextCh(); int ch1 = ch;
			NextCh(); int ch2 = ch;
			if (ch1 != 0xBB || ch2 != 0xBF) {
				throw new FatalError(String.Format("illegal byte order mark: EF {0,2:X} {1,2:X}", ch1, ch2));
			}
			buffer = new UTF8Buffer(buffer); col = 0;
			NextCh();
		}
		pt = tokens = new Token();  // first token is a dummy
	}
	
	void NextCh() {
		if (oldEols > 0) { ch = EOL; oldEols--; } 
		else {
			pos = buffer.Pos;
			ch = buffer.Read(); col++;
			// replace isolated '\r' by '\n' in order to make
			// eol handling uniform across Windows, Unix and Mac
			if (ch == '\r' && buffer.Peek() != '\n') ch = EOL;
			if (ch == EOL) { line++; col = 0; }
		}

	}

	void AddCh() {
		if (tlen >= tval.Length) {
			char[] newBuf = new char[2 * tval.Length];
			Array.Copy(tval, 0, newBuf, 0, tval.Length);
			tval = newBuf;
		}
		if (ch != Buffer.EOF) {
			tval[tlen++] = (char) ch;
			NextCh();
		}
	}



	bool Comment0() {
		int level = 1, pos0 = pos, line0 = line, col0 = col;
		NextCh();
		if (ch == '/') {
			NextCh();
			for(;;) {
				if (ch == 13) {
					NextCh();
					if (ch == 10) {
						level--;
						if (level == 0) { oldEols = line - line0; NextCh(); return true; }
						NextCh();
					}
				} else if (ch == Buffer.EOF) return false;
				else NextCh();
			}
		} else {
			buffer.Pos = pos0; NextCh(); line = line0; col = col0;
		}
		return false;
	}

	bool Comment1() {
		int level = 1, pos0 = pos, line0 = line, col0 = col;
		NextCh();
		if (ch == '*') {
			NextCh();
			for(;;) {
				if (ch == '*') {
					NextCh();
					if (ch == '/') {
						level--;
						if (level == 0) { oldEols = line - line0; NextCh(); return true; }
						NextCh();
					}
				} else if (ch == '/') {
					NextCh();
					if (ch == '*') {
						level++; NextCh();
					}
				} else if (ch == Buffer.EOF) return false;
				else NextCh();
			}
		} else {
			buffer.Pos = pos0; NextCh(); line = line0; col = col0;
		}
		return false;
	}


	void CheckLiteral() {
		switch (t.val) {
			case "sin": t.kind = 3; break;
			case "triangle": t.kind = 4; break;
			case "square": t.kind = 5; break;
			case "sawtooth": t.kind = 6; break;
			case "inversesawtooth": t.kind = 7; break;
			case "random": t.kind = 8; break;
			case "noise": t.kind = 9; break;
			case "GL_ZERO": t.kind = 10; break;
			case "GL_ONE": t.kind = 11; break;
			case "GL_SRC_COLOR": t.kind = 12; break;
			case "GL_ONE_MINUS_SRC_COLOR": t.kind = 13; break;
			case "GL_DST_COLOR": t.kind = 14; break;
			case "GL_ONE_MINUS_DST_COLOR": t.kind = 15; break;
			case "GL_SRC_ALPHA": t.kind = 16; break;
			case "GL_ONE_MINUS_SRC_ALPHA": t.kind = 17; break;
			case "GL_DST_ALPHA": t.kind = 18; break;
			case "GL_ONE_MINUS_DST_ALPHA": t.kind = 19; break;
			case "rgbGen": t.kind = 20; break;
			case "identityLighting": t.kind = 21; break;
			case "identity": t.kind = 22; break;
			case "entity": t.kind = 23; break;
			case "oneMinusEntity": t.kind = 24; break;
			case "vertex": t.kind = 25; break;
			case "oneMinusVertex": t.kind = 26; break;
			case "exactVertex": t.kind = 27; break;
			case "lightingDiffuse": t.kind = 28; break;
			case "LightingDiffuse": t.kind = 29; break;
			case "lightingSpecular": t.kind = 30; break;
			case "wave": t.kind = 31; break;
			case "const": t.kind = 32; break;
			case "alphaGen": t.kind = 35; break;
			case "portal": t.kind = 36; break;
			case "tcGen": t.kind = 37; break;
			case "base": t.kind = 38; break;
			case "lightmap": t.kind = 39; break;
			case "environment": t.kind = 40; break;
			case "vector": t.kind = 41; break;
			case "tcMod": t.kind = 42; break;
			case "rotate": t.kind = 43; break;
			case "scale": t.kind = 44; break;
			case "scroll": t.kind = 45; break;
			case "stretch": t.kind = 46; break;
			case "transform": t.kind = 47; break;
			case "turb": t.kind = 48; break;
			case "entityTranslate": t.kind = 49; break;
			case "alphaFunc": t.kind = 50; break;
			case "GT0": t.kind = 51; break;
			case "LT128": t.kind = 52; break;
			case "GE128": t.kind = 53; break;
			case "map": t.kind = 55; break;
			case "clampmap": t.kind = 59; break;
			case "animMap": t.kind = 60; break;
			case "blendFunc": t.kind = 61; break;
			case "add": t.kind = 62; break;
			case "filter": t.kind = 63; break;
			case "blend": t.kind = 64; break;
			case "depthFunc": t.kind = 65; break;
			case "equal": t.kind = 66; break;
			case "lequal": t.kind = 67; break;
			case "disable": t.kind = 68; break;
			case "depthWrite": t.kind = 69; break;
			case "detail": t.kind = 70; break;
			case "skyParms": t.kind = 72; break;
			case "skyparms": t.kind = 73; break;
			case "deformVertexes": t.kind = 75; break;
			case "normal": t.kind = 76; break;
			case "move": t.kind = 77; break;
			case "bulge": t.kind = 78; break;
			case "autoSprite": t.kind = 79; break;
			case "autoSprite2": t.kind = 80; break;
			case "projectionShadow": t.kind = 81; break;
			case "fogparms": t.kind = 82; break;
			case "surfaceparm": t.kind = 83; break;
			case "alphashadow": t.kind = 84; break;
			case "areaportal": t.kind = 85; break;
			case "clusterportal": t.kind = 86; break;
			case "donotenter": t.kind = 87; break;
			case "flesh": t.kind = 88; break;
			case "fog": t.kind = 89; break;
			case "lava": t.kind = 90; break;
			case "metalsteps": t.kind = 91; break;
			case "nodamage": t.kind = 92; break;
			case "nodlight": t.kind = 93; break;
			case "nodraw": t.kind = 94; break;
			case "nodrop": t.kind = 95; break;
			case "noimpact": t.kind = 96; break;
			case "nomarks": t.kind = 97; break;
			case "nolightmap": t.kind = 98; break;
			case "nosteps": t.kind = 99; break;
			case "nonsolid": t.kind = 100; break;
			case "origin": t.kind = 101; break;
			case "playerclip": t.kind = 102; break;
			case "slick": t.kind = 103; break;
			case "slime": t.kind = 104; break;
			case "structural": t.kind = 105; break;
			case "trans": t.kind = 106; break;
			case "water": t.kind = 107; break;
			case "pointlight": t.kind = 108; break;
			case "forcefield": t.kind = 109; break;
			case "shotclip": t.kind = 110; break;
			case "sky": t.kind = 111; break;
			case "monsterclip": t.kind = 112; break;
			case "hint": t.kind = 113; break;
			case "ladder": t.kind = 114; break;
			case "cull": t.kind = 115; break;
			case "front": t.kind = 116; break;
			case "back": t.kind = 117; break;
			case "none": t.kind = 118; break;
			case "sort": t.kind = 119; break;
			case "opaque": t.kind = 120; break;
			case "banner": t.kind = 121; break;
			case "underwater": t.kind = 122; break;
			case "additive": t.kind = 123; break;
			case "nearest": t.kind = 124; break;
			case "nopicmip": t.kind = 125; break;
			case "nomipmaps": t.kind = 126; break;
			case "polygonOffset": t.kind = 127; break;
			case "entityMergable": t.kind = 128; break;
			case "tessSize": t.kind = 129; break;
			case "q3map_backshader": t.kind = 130; break;
			case "q3map_globaltexture": t.kind = 131; break;
			case "q3map_sun": t.kind = 132; break;
			case "light": t.kind = 133; break;
			case "q3map_surfacelight": t.kind = 134; break;
			case "q3map_lightimage": t.kind = 135; break;
			case "q3map_lightsubdivide": t.kind = 136; break;
			case "q3map_backsplash": t.kind = 137; break;
			case "qer_editorimage": t.kind = 138; break;
			case "qer_nocarve": t.kind = 139; break;
			case "qer_trans": t.kind = 140; break;
			default: break;
		}
	}

	Token NextToken() {
		while (ch == ' ' ||
			ch >= 9 && ch <= 10 || ch == 13
		) NextCh();
		if (ch == '/' && Comment0() ||ch == '/' && Comment1()) return NextToken();
		t = new Token();
		t.pos = pos; t.col = col; t.line = line; 
		int state;
		if (start.ContainsKey(ch)) { state = (int) start[ch]; }
		else { state = 0; }
		tlen = 0; AddCh();
		
		switch (state) {
			case -1: { t.kind = eofSym; break; } // NextCh already done
			case 0: { t.kind = noSym; break; }   // NextCh already done
			case 1:
				if (ch >= '-' && ch <= '9' || ch >= 'A' && ch <= 'Z' || ch == '_' || ch >= 'a' && ch <= 'z') {AddCh(); goto case 1;}
				else {t.kind = 1; t.val = new String(tval, 0, tlen); CheckLiteral(); return t;}
			case 2:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 2;}
				else if (ch == '.') {AddCh(); goto case 3;}
				else {t.kind = 2; break;}
			case 3:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 3;}
				else {t.kind = 2; break;}
			case 4:
				{t.kind = 33; break;}
			case 5:
				{t.kind = 34; break;}
			case 6:
				{t.kind = 54; break;}
			case 7:
				if (ch == 'i') {AddCh(); goto case 8;}
				else {t.kind = noSym; break;}
			case 8:
				if (ch == 'g') {AddCh(); goto case 9;}
				else {t.kind = noSym; break;}
			case 9:
				if (ch == 'h') {AddCh(); goto case 10;}
				else {t.kind = noSym; break;}
			case 10:
				if (ch == 't') {AddCh(); goto case 11;}
				else {t.kind = noSym; break;}
			case 11:
				if (ch == 'm') {AddCh(); goto case 12;}
				else {t.kind = noSym; break;}
			case 12:
				if (ch == 'a') {AddCh(); goto case 13;}
				else {t.kind = noSym; break;}
			case 13:
				if (ch == 'p') {AddCh(); goto case 14;}
				else {t.kind = noSym; break;}
			case 14:
				{t.kind = 56; break;}
			case 15:
				if (ch == 'h') {AddCh(); goto case 16;}
				else {t.kind = noSym; break;}
			case 16:
				if (ch == 'i') {AddCh(); goto case 17;}
				else {t.kind = noSym; break;}
			case 17:
				if (ch == 't') {AddCh(); goto case 18;}
				else {t.kind = noSym; break;}
			case 18:
				if (ch == 'e') {AddCh(); goto case 19;}
				else {t.kind = noSym; break;}
			case 19:
				if (ch == 'i') {AddCh(); goto case 20;}
				else {t.kind = noSym; break;}
			case 20:
				if (ch == 'm') {AddCh(); goto case 21;}
				else {t.kind = noSym; break;}
			case 21:
				if (ch == 'a') {AddCh(); goto case 22;}
				else {t.kind = noSym; break;}
			case 22:
				if (ch == 'g') {AddCh(); goto case 23;}
				else {t.kind = noSym; break;}
			case 23:
				if (ch == 'e') {AddCh(); goto case 24;}
				else {t.kind = noSym; break;}
			case 24:
				{t.kind = 57; break;}
			case 25:
				if (ch == 'w') {AddCh(); goto case 26;}
				else {t.kind = noSym; break;}
			case 26:
				if (ch == 'h') {AddCh(); goto case 27;}
				else {t.kind = noSym; break;}
			case 27:
				if (ch == 'i') {AddCh(); goto case 28;}
				else {t.kind = noSym; break;}
			case 28:
				if (ch == 't') {AddCh(); goto case 29;}
				else {t.kind = noSym; break;}
			case 29:
				if (ch == 'e') {AddCh(); goto case 30;}
				else {t.kind = noSym; break;}
			case 30:
				{t.kind = 58; break;}
			case 31:
				{t.kind = 71; break;}
			case 32:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 2;}
				else {t.kind = 74; break;}
			case 33:
				if (ch == 'l') {AddCh(); goto case 7;}
				else if (ch == 'w') {AddCh(); goto case 15;}
				else {t.kind = noSym; break;}

		}
		t.val = new String(tval, 0, tlen);
		return t;
	}
	
	// get the next token (possibly a token already seen during peeking)
	public Token Scan () {
		if (tokens.next == null) {
			return NextToken();
		} else {
			pt = tokens = tokens.next;
			return tokens;
		}
	}

	// peek for the next token, ignore pragmas
	public Token Peek () {
		do {
			if (pt.next == null) {
				pt.next = NextToken();
			}
			pt = pt.next;
		} while (pt.kind > maxT); // skip pragmas
	
		return pt;
	}

	// make sure that peeking starts at the current scan position
	public void ResetPeek () { pt = tokens; }

} // end Scanner

}