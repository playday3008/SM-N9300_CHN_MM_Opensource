/*
 *	 The contents of this file are subject to the Netscape Public
 *	 License Version 1.1 (the "License"); you may not use this file
 *	 except in compliance with the License. You may obtain a copy of
 *	 the License at http://www.mozilla.org/NPL/
 *	
 *	 Software distributed under the License is distributed on an "AS
 *	 IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 *	 implied. See the License for the specific language governing
 *	 rights and limitations under the License.
 *	
 *	 The Original Code is the Netscape Messaging Access SDK Version 3.5 code, 
 *	released on or about June 15, 1998.  *	
 *	 The Initial Developer of the Original Code is Netscape Communications 
 *	 Corporation. Portions created by Netscape are
 *	 Copyright (C) 1998 Netscape Communications Corporation. All
 *	 Rights Reserved.
 */

/*
 * Copyright (c) 1997 and 1998 Netscape Communications Corporation
 * (http://home.netscape.com/misc/trademarks.html)
 */

package com.wssnps.base;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.text.TextUtils;

public class smlQP
{
    // error messages
    final static public String  szERROR_BAD_PARAMETER               = "Error: Bad parameter";
    final static public String  szERROR_OUT_OF_MEMORY               = "Error: Out of memory";
    final public static String  szERROR_EMPTY_MESSAGE               = "Error: Empty message";
    final public static String  szERROR_BAD_MIME_MESSAGE            = "Error: Bad mime message";
    final public static String  szERROR_BAD_EXTERNAL_MESSAGE_PART   = "Error:  No External headers in Message/external-body";
    final public static String  szERROR_UNSUPPORTED_PARTIAL_SUBTYPE = "Error: Unsupported Partial SubType";
    final public static String  szINVALID_CODE                      = "is an invalid code";
    protected static final int  BUFSZ                               = 4096;

    private static final byte   CR                                  = '\r';
    private static final byte   LF                                  = '\n';
    private static final byte   EQ                                  = '=';
    private static final byte   HT                                  = '\t';
    @SuppressWarnings("unused")
    private static final byte[] CRLF                                = "\r\n".getBytes();
    private static final byte[] EQCRLF                              = "=\r\n".getBytes();

    private static final byte   hexmap[]                            = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    /**
     * QuotedPrintable Encodes data from InputStream and writes to OutputStream.
     * 
     * @param input
     *            InputStream that supplies the data to be encoded.
     * @param output
     *            OutputStream that accepts the encoded data.
     * @return Number of bytes written.
     * @exception MimeException
     *                If an encoding error occurs.
     * @exception IOException
     *                If an I/O error occurs.
     */
    public static long encodeQP(InputStream input, ByteArrayOutputStream output) throws smlMimeException, IOException
    {
        byte current = (byte) 0, previous = (byte) 0;
        int read, linelen = 0, written = 0, lastspace = 0, nullCount = 0;
        byte l_bufenc[] = new byte[80];

        while (true)
        {
            read = input.read();

            if (read == -1)
            {
                if (linelen > 0)
                {
                    output.write(l_bufenc, 0, linelen);
                    // output.write (CRLF);
                    // written += 2;
                }

                return (written);
            }

            if (linelen > 74)
            {
                output.write(l_bufenc, 0, linelen);
                // output.write (EQCRLF);
                // written += 3;
                linelen = 0;
                previous = (byte) 0;
            }
            current = (byte) read;

            if (current == 0x00)
            {
                nullCount++;
                previous = current;
                lastspace = 0;
                continue;
            }
            else if (nullCount > 0)
            {
                // write out all the nulls first and fall through to process current char.
                for (int idx = 1; idx <= nullCount; idx++)
                {
                    byte tmp = 0x00;
                    l_bufenc[linelen++] = EQ;
                    l_bufenc[linelen++] = (byte) hexmap[(tmp >>> 4) & 0xF];
                    l_bufenc[linelen++] = (byte) hexmap[(tmp & 0xF)];
                    // l_bufenc [linelen++] = (byte)0x00;
                    // l_bufenc [linelen++] = (byte)0x00;
                    written += 3;

                    if (linelen > 74)
                    {
                        output.write(l_bufenc, 0, linelen);
                        // output.write (EQCRLF);
                        // written += 3;
                        linelen = 0;
                    }
                }

                previous = (byte) 0;
                nullCount = 0;
            }

            if ((current > ' ') && (current < 0x7F) && (current != '=') && (current != '(') && (current != ')') && (current != ';') && (current != ':'))
            {
                // Printable chars
                // output.write ((byte) current);
                l_bufenc[linelen++] = (byte) current;
                // linelen += 1;
                written += 1;
                lastspace = 0;
                previous = current;
            }
            else if ((current == ' ') || (current == HT))
            {
                // output.write ((byte) current);
                l_bufenc[linelen++] = (byte) current;
                written += 1;
                lastspace = 1;
                previous = current;
            }
            else if ((current == LF) && (previous == CR))
            {
                // handled this already. Ignore.
                previous = (byte) 0;
            }
            else if (current == LF)
            {
                if ((lastspace == 1) || ((previous == '.') && (linelen == 1)))
                {
                    l_bufenc[linelen++] = EQ;
                    l_bufenc[linelen++] = CR;
                    l_bufenc[linelen++] = LF;
                    written += 3;
                }

                l_bufenc[linelen++] = EQ;
                l_bufenc[linelen++] = '0';
                l_bufenc[linelen++] = 'A';

                lastspace = 0;
                written += 3;

                output.write(l_bufenc, 0, linelen);
                previous = (byte) 0;
                linelen = 0;
            }
            else if ((current == CR))// || (current == LF))
            {
                // Need to emit a soft line break if last char was SPACE/HT or
                // if we have a period on a line by itself.
                if ((lastspace == 1) || ((previous == '.') && (linelen == 1)))
                {
                    l_bufenc[linelen++] = EQ;
                    l_bufenc[linelen++] = CR;
                    l_bufenc[linelen++] = LF;
                    written += 3;
                }

//                l_bufenc[linelen++] = CR;
//                l_bufenc[linelen++] = LF;
                
                l_bufenc[linelen++] = EQ;
                l_bufenc[linelen++] = '0';
                l_bufenc[linelen++] = 'D';

                lastspace = 0;
//                written += 2;
                written += 3;
                output.write(l_bufenc, 0, linelen);
                previous = (byte) 0;
                linelen = 0;
                // output.write (CRLF);
                // previous = current;
            }
            else if ((current < ' ') || (current == '=') || (current >= 0x7F) || (current == '(') || (current == ')') || (current == ';') || (current == ':'))
            {
                // Special Chars
                // output.write ((byte) '=');
                // output.write ((byte) hexmap [(current >>> 4)]);
                // output.write ((byte) hexmap [(current & 0xF)]);
                l_bufenc[linelen++] = EQ;

                l_bufenc[linelen++] = (byte) hexmap[(current >>> 4) & 0xF];
                l_bufenc[linelen++] = (byte) hexmap[(current & 0xF)];
                lastspace = 0;
                // linelen += 3;
                written += 3;
                previous = current;
            }
            else
            {
                // output.write ((byte) current);
                l_bufenc[linelen++] = (byte) current;
                lastspace = 0;
                // linelen += 1;
                written += 1;
                previous = current;
            }
        } // while

    } // encodeQP

    public static String encodeQP(String in) throws smlMimeException
    {
        ByteArrayInputStream input = new ByteArrayInputStream(in.getBytes());
        ByteArrayOutputStream output = new ByteArrayOutputStream();
        try
        {
            encodeQP(input, output);
        }
        catch (IOException e)
        {
            e.printStackTrace();
        }
        String ret = output.toString();
        return ret;
    }

    /**
     * QuotedPrintable Decodes data from InputStream and writes to OutputStream
     * 
     * @param input
     *            InputStream that supplies the data to be decoded.
     * @param output
     *            OutputStream that accepts the decoded data.
     * @exception smlMimeException
     *                if a decoding error occurs.
     * @exception IOException
     *                if io error occurs.
     */
    public static void decodeQP(InputStream input, OutputStream output) throws smlMimeException
    {
        byte inputBuffer[];

        try
        {
            inputBuffer = new byte[input.available() + 1];
            input.read(inputBuffer);
            output.write(decodeQP(inputBuffer));
        }
        catch (IOException e)
        {
            throw new smlMimeException(e.getMessage());
        }
        catch (Exception e)
        {
            throw new smlMimeException(e.getMessage());
        }

        return;
    }

    /**
     * String version. QP decodes data from the input string.
     * 
     * @param str
     *            the QP-encoded string.
     * @return the decoded string
     */
    public static String decodeQP(String str) throws smlMimeException
    {
        String decodedString = "";
        if (TextUtils.isEmpty(str))
            return decodedString;

        try
        {
            decodedString = new String(decodeQP(str.getBytes()));
        }
        catch (Exception e)
        {
            throw new smlMimeException(e.getMessage());
        }
        return decodedString;
    }

    /**
     * byte[] version. QP decodes input bytes.
     * 
     * @param bytesIn
     *            QP-encoded bytes.
     * @return the decoded bytes
     */
    protected static byte[] decodeQP(byte[] bytesIn) throws smlMimeException
    {
        return decodeQP(bytesIn, bytesIn.length);
    }

    /**
     * byte[] version. QP decodes input bytes of given length.
     * 
     * @param bytesIn
     *            QP-encoded bytes.
     * @param len
     *            length of QP-encoded bytes.
     * @return the decoded bytes
     * @exception ParseException
     *                If a '=' is not followed by a valid 2-digit hex number or '\r\n'.
     */
    /**
     * 2.4 Quoted-Printable Encoding 규칙 다음은 RFC2045에서 정의한 Quoted-Printable Encoding 규칙이다.
     * 
     * [규칙 1] 모든 옥텟(바이트)의 인코딩은 그 값을 16진수로 표현하여 '=' 뒤에 붙이면 된다. 옥텟의 값을 16진수로 표현하는데 사용되는 문자는 0123456789ABCDEF 이며, 대문자만을 사용한다. 예를 들어 십진수로 12인 옥텟(LF)을
     * Quoted-Printable로 인코딩하면 "=0C"가 되고, 십진수로 61인 옥텟(=)은 "=3D"가 되는 것이다. 뒤에 나오는 규칙 2-5에서 제시되고 있는 인코딩 방법을 사용하지 않는 모든 옥텟은 이 방식으로 인코딩 해야 한다.
     * 
     * [규칙 2] 십진수 33 - 60, 62 - 126의 문자(제어코드와 십진수 61인 '='를 제외한 아스키 값)는 규칙 1에 따라 인코딩 되지 않아도 된다. 즉, 이스케이프 문자인 '='를 제외한 영문자와 숫자등은 그대로 표현된다.
     * 
     * [규칙 3] 십진수 9(TAB)와 32(SPACE)는 절대로 인코딩 된 문자열의 끝에 나타나서는 않된다. 그러므로 문자열의 끝에 있는 Tab이나 Space는 규칙 1에 따라 인코딩 되어야 한다.
     * 
     * [규칙 4] 줄바꿈 문자(컴퓨터에 따라 CR, LF, CRLF로 각기 달리 나타나는 문자)는 CRLF의 형태로 표현되어야 한다.(RFC822 정의)
     * 
     * [규칙 5] Quoted-Printable로 인코딩 된 문자열의 길이는 76자 이상 이어서는 않된다. 이 때 문자열의 길이는 문자열 맨 뒤에 붙는 CRLF는 제외한 나머지 문자열의 길이를 말한다. 인코딩 되었을 때, 76자가 넘는 문자열에는 Soft
     * Line Break를 사용한다. 즉 원래의 문자열에는 영향을 주지 않고 단지 인코딩 된 문자열의 줄바꿈만을 나타내는 문자를 덧붙이고 줄바꿈을 하여, 인코딩 된 문자열이 76자가 넘지 않도록 하는 것이다. Quoted-Printable에서는 '='을 Soft
     * Line Break로 사용한다. Soft Line Break 뒤에는 Tab이나 Space가 나타날 수 있으며 규칙 3에 의한 인코딩 해서는 않된다. 그 이유는 몇몇 MTA들이 전송하는 원래의 문자열에 Tab이나 Space를 붙이기도 하고 빼기도 하는데, 이
     * 때 덧붙여지는 Tab이나 Space는 원래의 데이터가 아니기 때문이다.
     */
    protected static byte[] decodeQP(byte[] bytesIn, int len) throws smlMimeException
    {
        if (bytesIn == null)
            throw new smlMimeException(szERROR_BAD_PARAMETER);

        byte res[] = new byte[len + 1];
        byte src[] = bytesIn;
        byte nl[] = "\r\n".getBytes(); // System.getProperty("line.separator").getBytes();

        int last = 0, j = 0;

        for (int i = 0; i < len;)
        {
            byte ch = src[i++];
            if (ch == '=')
            {
                if (src[i] == '\n' || src[i] == '\r')
                { // Rule #5
                    i++;

                    if (src[i - 1] == '\r' && src[i] == '\n')
                        i++;
                }
                else
                // Rule #1
                {
                    byte repl;
                    int hi = Character.digit((char) src[i], 16);
                    int lo = Character.digit((char) src[i + 1], 16);

                    if ((hi | lo) < 0)
                    {
                        throw new smlMimeException(new String(src, i - 1, 3) + szINVALID_CODE);
                    }
                    else
                    {
                        repl = (byte) (hi << 4 | lo);
                        i += 2;
                    }

                    res[j++] = repl;

                }
                last = j;
            }
            else if (ch == '\n' || ch == '\r') // Rule #4
            {
                if (src[i - 1] == '\r' && src[i] == '\n')
                    i++;

                for (int idx = 0; idx < nl.length; idx++)
                    res[last++] = nl[idx];

                j = last;
            }
            else
            // Rule #1, #2
            {
                res[j++] = ch;

                if (ch != ' ' && ch != '\t') // Rule #3
                    last = j;
            }
        }

        byte res2[] = new byte[j];
        int k = 0;
        for (k = 0; k < j; k++)
        {
            res2[k] = res[k];
        }
        return res2;
    }
}