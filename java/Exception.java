/*
 * Corresponds to the exception.h file in Jarvis.
 */

package jarvis;

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
