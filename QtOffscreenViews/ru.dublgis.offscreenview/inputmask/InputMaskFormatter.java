package ru.dublgis.offscreenview.inputmask;

import ru.dublgis.androidhelpers.Log;

import java.util.HashMap;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

/*! 
 * Some simple analog of https://doc.qt.io/qt-5/qlineedit.html#inputMask-prop
 * Note: Escape sequences (><{}\\[]) not supported
 */
public class InputMaskFormatter {

    protected String mask_;
    protected HashMap<String, MaskPattern> patternsMap_;
    protected HashMap<String, MaskPattern> maskPatterns_;

    public enum ValidationState {
        Invalid,
        Intermediate,
        Acceptable
    }

    public class MaskPattern 
    {
        private Pattern pattern_;

        public MaskPattern(final String regex) 
        {
            try 
            {
                pattern_ = Pattern.compile(regex);
            } 
            catch (final Throwable e) 
            {
                Log.w("MaskPattern: '" + regex + "' exception: ", e);
            }
        }

        public boolean isValid(final char c) 
        {
            final String value = transform(c);
            return isValid(value);
        }

        public boolean isValid(final String s) 
        {
            final Matcher matcher = pattern_.matcher(s);
            return matcher.matches();
        }

        public String transform(char c) 
        {
            return String.valueOf(c);
        }
    }

    public InputMaskFormatter(final String mask) 
    {
        initPatterns();
        setMask(mask);
    }

    public void setMask(final String mask) 
    {
        if (mask_ != mask) 
        {
            mask_ = mask;
            parseMask(mask);
        }
    }

    private void initPatterns()
    {
        patternsMap_ = new HashMap<>();

        patternsMap_.put("A", new MaskPattern("\\p{Alpha}"));
        patternsMap_.put("a", new MaskPattern("\\p{Alpha}?"));
        patternsMap_.put("N", new MaskPattern("\\p{Alnum}"));
        patternsMap_.put("n", new MaskPattern("\\p{Alnum}?"));
        patternsMap_.put("X", new MaskPattern("\\p{Blank}"));
        patternsMap_.put("x", new MaskPattern("\\p{Blank}?"));
        patternsMap_.put("9", new MaskPattern("\\p{Digit}"));
        patternsMap_.put("0", new MaskPattern("\\p{Digit}?"));
        patternsMap_.put("D", new MaskPattern("\\d"));
        patternsMap_.put("d", new MaskPattern("\\d?"));
        patternsMap_.put("H", new MaskPattern("\\p{XDigit}"));
        patternsMap_.put("h", new MaskPattern("\\p{XDigit}?"));
    }

    private void parseMask(final String mask) 
    {
        this.maskPatterns_ = new HashMap<>();
        for (int i = 0; i < mask.length(); i++)
        {
            final String key = String.valueOf(mask.charAt(i));
            final MaskPattern pattern = patternsMap_.get(key);
            if (pattern != null) 
            {
                maskPatterns_.put(key, pattern);
            }
        }
    }

    public String format(final String text) 
    {
        if (text == null || "".equals(text))
        {
            return "";
        }

        int offset = 0;
        StringBuilder result = new StringBuilder();

        for (int i = 0; i < mask_.length(); i++) 
        {
            final String patternKey = String.valueOf(mask_.charAt(i));
            final MaskPattern pattern = maskPatterns_.get(patternKey); 
            if (pattern != null)
            {
                final int nextIndex = indexOfFirstValidChar(pattern, text, offset);
                if (nextIndex >= 0 && nextIndex < text.length()) 
                {
                    offset = nextIndex + 1;

                    final char nextChar = text.charAt(nextIndex);
                    result.append(pattern.transform(nextChar));

                    if (offset >= text.length())
                    {
                        return result.toString();
                    }
                } 
                else 
                {
                    return result.toString();
                } 
            } 
            else
            {
                result.append(mask_.charAt(i));
            }
        }

        return result.toString();
    }

    public boolean acceptableInput(final String text) 
    {
        final ValidationState validationState = validate(text);
        return validationState == ValidationState.Acceptable;
    }

    public ValidationState validate(final String text) 
    {
        ValidationState state = ValidationState.Invalid;

        if (text == null || text.isEmpty() || mask_ == null || mask_.isEmpty()) 
        {
            return state;
        }

        if (text.length() > mask_.length()) 
        {
            return state;
        }

        for (int i = 0; i < mask_.length(); i++) 
        {
            final char ch = i < text.length() ? text.charAt(i)
                                              : '\0';

            //NOTE: convertation empty char '\0' to string not match with patterns with '?'
            final String s = i < text.length() ? String.valueOf(ch) 
                                               : "";

            final String key = String.valueOf(mask_.charAt(i));
                
            if (s.equals(key))
            {
                state = ValidationState.Intermediate;
                continue;
            }

            final MaskPattern p = maskPatterns_.get(key);
            if (p != null)
            {
                if (p.isValid(s)) 
                {
                    state = (i == mask_.length() - 1) ? ValidationState.Acceptable
                                                      : ValidationState.Intermediate;
                    continue;
                } 
                else 
                {
                    if (i >= text.length()) 
                    {
                        state = ValidationState.Intermediate;
                        break;
                    }
                }
            }
            else 
            {
                if (i >= text.length()) 
                {
                    state = ValidationState.Intermediate;
                    break;
                }

                state = ValidationState.Invalid;
                break;
            }
        }

        return state;
    }

    public String mask()
    {
        return mask_;
    }

    private int indexOfFirstValidChar(final MaskPattern pattern, final String text, int offset) 
    {
        for (int i = offset; i < text.length(); i++) 
        {
            final char c = text.charAt(i);
            if (pattern.isValid(c))
            {
                return i;
            }
        }
        return -1;
    }
}
