/**
 * @file   Exception.java
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/*
 * Corresponds to the exception.h file in PMGD.
 */

package pmgd;

public class Exception extends java.lang.Exception
{
    public int num;
    public String name;

    public String msg;
    public int errno_val;
    public String errno_msg;

    public String file;
    public int line;
    public String function;

    public Exception(int num, String name, String msg,
                     int errno_val, String errno_msg,
                     String file, int line)
    {
        this.num = num;
        this.name = name;
        this.msg = msg;
        this.errno_val = errno_val;
        this.errno_msg = errno_msg;
        this.file = file;
        this.line = line;
    }

    public Exception(int num, String name, String function, String msg)
    {
        this.num = num;
        this.name = name;
        this.msg = msg;
        this.function = function;
    }

    public void print()
    {
        if (function != null)
            System.out.printf("[Exception] %s in %s\n", name, function);
        else
            System.out.printf("[Exception] %s at %s:%d\n", name, file, line);
        if (errno_val != 0)
            System.out.printf("%s: %s (%d)\n", msg, errno_msg, errno_val);
        else if (msg != null)
            System.out.printf("%s\n", msg);
    }
}
