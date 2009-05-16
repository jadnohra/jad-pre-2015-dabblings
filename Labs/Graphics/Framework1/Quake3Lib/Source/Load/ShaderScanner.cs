
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
	const int maxT = 209;
	const int noSym = 209;
	char valCh;       // current input character (for token.val)

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
		for (int i = 97; i <= 122; ++i) start[i] = 1;
		for (int i = 48; i <= 57; ++i) start[i] = 2;
		start[45] = 38; 
		start[46] = 7; 
		start[40] = 10; 
		start[41] = 11; 
		start[123] = 12; 
		start[36] = 39; 
		start[42] = 31; 
		start[125] = 37; 
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
		if (ch != Buffer.EOF) {
			valCh = (char) ch;
			ch = char.ToLower((char) ch);
		}

	}

	void AddCh() {
		if (tlen >= tval.Length) {
			char[] newBuf = new char[2 * tval.Length];
			Array.Copy(tval, 0, newBuf, 0, tval.Length);
			tval = newBuf;
		}
		if (ch != Buffer.EOF) {
			tval[tlen++] = valCh;
			NextCh();
		}
	}



	bool Comment0() {
		int level = 1, pos0 = pos, line0 = line, col0 = col;
		NextCh();
		if (ch == '/') {
			NextCh();
			for(;;) {
				if (ch == 10) {
					level--;
					if (level == 0) { oldEols = line - line0; NextCh(); return true; }
					NextCh();
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
		if (ch == '/') {
			NextCh();
			for(;;) {
				if (ch == 13) {
					level--;
					if (level == 0) { oldEols = line - line0; NextCh(); return true; }
					NextCh();
				} else if (ch == Buffer.EOF) return false;
				else NextCh();
			}
		} else {
			buffer.Pos = pos0; NextCh(); line = line0; col = col0;
		}
		return false;
	}

	bool Comment2() {
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
		switch (t.val.ToLower()) {
			case "sin": t.kind = 3; break;
			case "triangle": t.kind = 4; break;
			case "square": t.kind = 5; break;
			case "sawtooth": t.kind = 6; break;
			case "inversesawtooth": t.kind = 7; break;
			case "random": t.kind = 8; break;
			case "noise": t.kind = 9; break;
			case "dotproduct": t.kind = 10; break;
			case "dotproduct2": t.kind = 13; break;
			case "dotproductscale": t.kind = 14; break;
			case "dotproduct2scale": t.kind = 15; break;
			case "scale": t.kind = 16; break;
			case "set": t.kind = 17; break;
			case "vector": t.kind = 18; break;
			case "ivector": t.kind = 19; break;
			case "rotate": t.kind = 20; break;
			case "translate": t.kind = 21; break;
			case "shift": t.kind = 22; break;
			case "move": t.kind = 23; break;
			case "gl_zero": t.kind = 24; break;
			case "gl_one": t.kind = 25; break;
			case "gl_src_color": t.kind = 26; break;
			case "gl_one_minus_src_color": t.kind = 27; break;
			case "gl_dst_color": t.kind = 28; break;
			case "gl_one_minus_dst_color": t.kind = 29; break;
			case "gl_src_alpha": t.kind = 30; break;
			case "gl_one_minus_src_alpha": t.kind = 31; break;
			case "gl_dst_alpha": t.kind = 32; break;
			case "gl_one_minus_dst_alpha": t.kind = 33; break;
			case "rgbgen": t.kind = 34; break;
			case "identitylighting": t.kind = 35; break;
			case "identity": t.kind = 36; break;
			case "entity": t.kind = 37; break;
			case "oneminusentity": t.kind = 38; break;
			case "vertex": t.kind = 39; break;
			case "oneminusvertex": t.kind = 40; break;
			case "exactvertex": t.kind = 41; break;
			case "lightingdiffuse": t.kind = 42; break;
			case "lightingspecular": t.kind = 43; break;
			case "wave": t.kind = 44; break;
			case "const": t.kind = 45; break;
			case "alphagen": t.kind = 46; break;
			case "portal": t.kind = 47; break;
			case "tcgen": t.kind = 48; break;
			case "base": t.kind = 49; break;
			case "lightmap": t.kind = 50; break;
			case "environment": t.kind = 51; break;
			case "tcmod": t.kind = 52; break;
			case "scroll": t.kind = 53; break;
			case "stretch": t.kind = 54; break;
			case "transform": t.kind = 55; break;
			case "turb": t.kind = 56; break;
			case "entitytranslate": t.kind = 57; break;
			case "alphafunc": t.kind = 58; break;
			case "gt0": t.kind = 59; break;
			case "lt128": t.kind = 60; break;
			case "ge128": t.kind = 61; break;
			case "map": t.kind = 63; break;
			case "clampmap": t.kind = 67; break;
			case "animmap": t.kind = 68; break;
			case "blendfunc": t.kind = 69; break;
			case "add": t.kind = 70; break;
			case "filter": t.kind = 71; break;
			case "blend": t.kind = 72; break;
			case "depthfunc": t.kind = 73; break;
			case "equal": t.kind = 74; break;
			case "lequal": t.kind = 75; break;
			case "disable": t.kind = 76; break;
			case "depthwrite": t.kind = 77; break;
			case "detail": t.kind = 78; break;
			case "skyparms": t.kind = 80; break;
			case "deformvertexes": t.kind = 82; break;
			case "normal": t.kind = 83; break;
			case "bulge": t.kind = 84; break;
			case "autosprite": t.kind = 85; break;
			case "autosprite2": t.kind = 86; break;
			case "projectionshadow": t.kind = 87; break;
			case "fogparms": t.kind = 88; break;
			case "surfaceparm": t.kind = 89; break;
			case "alphashadow": t.kind = 90; break;
			case "areaportal": t.kind = 91; break;
			case "clusterportal": t.kind = 92; break;
			case "donotenter": t.kind = 93; break;
			case "flesh": t.kind = 94; break;
			case "fog": t.kind = 95; break;
			case "lava": t.kind = 96; break;
			case "metalsteps": t.kind = 97; break;
			case "nodamage": t.kind = 98; break;
			case "nodlight": t.kind = 99; break;
			case "nodraw": t.kind = 100; break;
			case "nodrop": t.kind = 101; break;
			case "noimpact": t.kind = 102; break;
			case "nomarks": t.kind = 103; break;
			case "nolightmap": t.kind = 104; break;
			case "nosteps": t.kind = 105; break;
			case "nonsolid": t.kind = 106; break;
			case "origin": t.kind = 107; break;
			case "playerclip": t.kind = 108; break;
			case "slick": t.kind = 109; break;
			case "slime": t.kind = 110; break;
			case "structural": t.kind = 111; break;
			case "trans": t.kind = 112; break;
			case "water": t.kind = 113; break;
			case "pointlight": t.kind = 114; break;
			case "forcefield": t.kind = 115; break;
			case "shotclip": t.kind = 116; break;
			case "sky": t.kind = 117; break;
			case "monsterclip": t.kind = 118; break;
			case "hint": t.kind = 119; break;
			case "ladder": t.kind = 120; break;
			case "dust": t.kind = 121; break;
			case "botclip": t.kind = 122; break;
			case "antiportal": t.kind = 123; break;
			case "lightgrid": t.kind = 124; break;
			case "cull": t.kind = 125; break;
			case "front": t.kind = 126; break;
			case "back": t.kind = 127; break;
			case "none": t.kind = 128; break;
			case "sort": t.kind = 129; break;
			case "opaque": t.kind = 130; break;
			case "banner": t.kind = 131; break;
			case "underwater": t.kind = 132; break;
			case "additive": t.kind = 133; break;
			case "nearest": t.kind = 134; break;
			case "nopicmip": t.kind = 135; break;
			case "nomipmaps": t.kind = 136; break;
			case "polygonoffset": t.kind = 137; break;
			case "entitymergable": t.kind = 138; break;
			case "tesssize": t.kind = 139; break;
			case "light": t.kind = 140; break;
			case "qer_editorimage": t.kind = 141; break;
			case "qer_nocarve": t.kind = 142; break;
			case "qer_trans": t.kind = 143; break;
			case "q3map_alphagen": t.kind = 144; break;
			case "q3map_alphamod": t.kind = 145; break;
			case "q3map_backshader": t.kind = 146; break;
			case "q3map_backsplash": t.kind = 147; break;
			case "q3map_baseshader": t.kind = 148; break;
			case "q3map_bounce": t.kind = 149; break;
			case "q3map_bouncescale": t.kind = 150; break;
			case "q3map_clipmodel": t.kind = 151; break;
			case "q3map_cloneshader": t.kind = 152; break;
			case "q3map_colorgen": t.kind = 153; break;
			case "q3map_colormod": t.kind = 154; break;
			case "q3map_fogdir": t.kind = 155; break;
			case "q3map_forcemeta": t.kind = 156; break;
			case "q3map_forcesunlight": t.kind = 157; break;
			case "q3map_fur": t.kind = 158; break;
			case "q3map_globaltexture": t.kind = 159; break;
			case "q3map_indexed": t.kind = 160; break;
			case "q3map_invert": t.kind = 161; break;
			case "q3map_lightimage": t.kind = 162; break;
			case "q3map_lightmapaxis": t.kind = 163; break;
			case "q3map_lightmapbrightness": t.kind = 164; break;
			case "q3map_lightmapfilterradius": t.kind = 165; break;
			case "q3map_lightmapgamma": t.kind = 166; break;
			case "q3map_lightmapmergable": t.kind = 167; break;
			case "q3map_lightmapsampleoffset": t.kind = 168; break;
			case "q3map_lightmapsamplesize": t.kind = 169; break;
			case "q3map_lightmapsize": t.kind = 170; break;
			case "q3map_lightrgb": t.kind = 171; break;
			case "q3map_lightstyle": t.kind = 172; break;
			case "q3map_lightsubdivide": t.kind = 173; break;
			case "q3map_noclip": t.kind = 174; break;
			case "q3map_nofast": t.kind = 175; break;
			case "q3map_nofog": t.kind = 176; break;
			case "q3map_nonplanar": t.kind = 177; break;
			case "q3map_normalimage": t.kind = 178; break;
			case "q3map_notjunc": t.kind = 179; break;
			case "q3map_novertexlight": t.kind = 180; break;
			case "q3map_novertexshadows": t.kind = 181; break;
			case "q3map_offset": t.kind = 182; break;
			case "q3map_patchshadows": t.kind = 183; break;
			case "q3map_remapshader": t.kind = 184; break;
			case "q3map_replicate": t.kind = 185; break;
			case "q3map_rgbgen": t.kind = 186; break;
			case "q3map_rgbmod": t.kind = 187; break;
			case "q3map_shadeangle": t.kind = 188; break;
			case "q3map_skylight": t.kind = 189; break;
			case "q3map_splotchfix": t.kind = 190; break;
			case "q3map_stylemarker": t.kind = 191; break;
			case "q3map_stylemarker2": t.kind = 192; break;
			case "q3map_sun": t.kind = 193; break;
			case "q3map_sunext": t.kind = 194; break;
			case "q3map_sunlight": t.kind = 195; break;
			case "q3map_surfacelight": t.kind = 196; break;
			case "q3map_surfacemodel": t.kind = 197; break;
			case "q3map_tcgen": t.kind = 198; break;
			case "q3map_tcmod": t.kind = 199; break;
			case "q3map_terrain": t.kind = 200; break;
			case "q3map_tesssize": t.kind = 201; break;
			case "q3map_texturesize": t.kind = 202; break;
			case "q3map_tracelight": t.kind = 203; break;
			case "q3map_vertexscale": t.kind = 204; break;
			case "q3map_vertexshadows": t.kind = 205; break;
			case "q3map_vlight": t.kind = 206; break;
			case "q3map_flare": t.kind = 207; break;
			case "q3map_nolightmap": t.kind = 208; break;
			default: break;
		}
	}

	Token NextToken() {
		while (ch == ' ' ||
			ch >= 9 && ch <= 10 || ch == 13
		) NextCh();
		if (ch == '/' && Comment0() ||ch == '/' && Comment1() ||ch == '/' && Comment2()) return NextToken();
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
				if (ch >= '-' && ch <= '9' || ch == '_' || ch >= 'a' && ch <= 'z') {AddCh(); goto case 1;}
				else {t.kind = 1; t.val = new String(tval, 0, tlen); CheckLiteral(); return t;}
			case 2:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 2;}
				else if (ch == '.') {AddCh(); goto case 3;}
				else {t.kind = 2; break;}
			case 3:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 3;}
				else {t.kind = 2; break;}
			case 4:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 5;}
				else {t.kind = noSym; break;}
			case 5:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 6;}
				else {t.kind = 2; break;}
			case 6:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 6;}
				else {t.kind = 2; break;}
			case 7:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 8;}
				else {t.kind = noSym; break;}
			case 8:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 9;}
				else {t.kind = 2; break;}
			case 9:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 9;}
				else {t.kind = 2; break;}
			case 10:
				{t.kind = 11; break;}
			case 11:
				{t.kind = 12; break;}
			case 12:
				{t.kind = 62; break;}
			case 13:
				if (ch == 'i') {AddCh(); goto case 14;}
				else {t.kind = noSym; break;}
			case 14:
				if (ch == 'g') {AddCh(); goto case 15;}
				else {t.kind = noSym; break;}
			case 15:
				if (ch == 'h') {AddCh(); goto case 16;}
				else {t.kind = noSym; break;}
			case 16:
				if (ch == 't') {AddCh(); goto case 17;}
				else {t.kind = noSym; break;}
			case 17:
				if (ch == 'm') {AddCh(); goto case 18;}
				else {t.kind = noSym; break;}
			case 18:
				if (ch == 'a') {AddCh(); goto case 19;}
				else {t.kind = noSym; break;}
			case 19:
				if (ch == 'p') {AddCh(); goto case 20;}
				else {t.kind = noSym; break;}
			case 20:
				{t.kind = 64; break;}
			case 21:
				if (ch == 'h') {AddCh(); goto case 22;}
				else {t.kind = noSym; break;}
			case 22:
				if (ch == 'i') {AddCh(); goto case 23;}
				else {t.kind = noSym; break;}
			case 23:
				if (ch == 't') {AddCh(); goto case 24;}
				else {t.kind = noSym; break;}
			case 24:
				if (ch == 'e') {AddCh(); goto case 25;}
				else {t.kind = noSym; break;}
			case 25:
				if (ch == 'i') {AddCh(); goto case 26;}
				else {t.kind = noSym; break;}
			case 26:
				if (ch == 'm') {AddCh(); goto case 27;}
				else {t.kind = noSym; break;}
			case 27:
				if (ch == 'a') {AddCh(); goto case 28;}
				else {t.kind = noSym; break;}
			case 28:
				if (ch == 'g') {AddCh(); goto case 29;}
				else {t.kind = noSym; break;}
			case 29:
				if (ch == 'e') {AddCh(); goto case 30;}
				else {t.kind = noSym; break;}
			case 30:
				{t.kind = 65; break;}
			case 31:
				if (ch == 'w') {AddCh(); goto case 32;}
				else {t.kind = noSym; break;}
			case 32:
				if (ch == 'h') {AddCh(); goto case 33;}
				else {t.kind = noSym; break;}
			case 33:
				if (ch == 'i') {AddCh(); goto case 34;}
				else {t.kind = noSym; break;}
			case 34:
				if (ch == 't') {AddCh(); goto case 35;}
				else {t.kind = noSym; break;}
			case 35:
				if (ch == 'e') {AddCh(); goto case 36;}
				else {t.kind = noSym; break;}
			case 36:
				{t.kind = 66; break;}
			case 37:
				{t.kind = 79; break;}
			case 38:
				if (ch >= '0' && ch <= '9') {AddCh(); goto case 2;}
				else if (ch == '.') {AddCh(); goto case 4;}
				else {t.kind = 81; break;}
			case 39:
				if (ch == 'l') {AddCh(); goto case 13;}
				else if (ch == 'w') {AddCh(); goto case 21;}
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