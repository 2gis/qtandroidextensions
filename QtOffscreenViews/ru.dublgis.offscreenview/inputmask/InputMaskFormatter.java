package ru.dublgis.offscreenview.inputmask;

import ru.dublgis.androidhelpers.Log;

import java.util.HashMap;
import java.util.regex.Pattern;
import java.util.regex.Matcher;

public class InputMaskFormatter {

    protected String mask;
    protected HashMap<String, MaskPattern> patternMap;

    public class MaskPattern 
    {
        private Pattern pattern;
        private String regex;

        public MaskPattern(String regex) 
        {
            this.pattern = Pattern.compile(regex);
            this.regex = regex;
        }

        public boolean isValid(final char c) 
        {
            final Matcher matcher = pattern.matcher(transform(c));
            return matcher.matches();
        }

        public String transform(char c) 
        {
            return String.valueOf(c);
        }
    }

    public InputMaskFormatter(String mask) 
    {
        this.patternMap = new HashMap<>();
        this.mask = mask;
    }

    public void setMask(String mask) 
    {
        this.mask = mask;
    }

    public void registerPattern(String key, String regex) 
    {
        try 
        {
            patternMap.put(key, new MaskPattern(regex));   
        } 
        catch (final Throwable e)
        {
            Log.w("InputMaskFormatter.registerPattern: '" + regex + "' exception: ", e);
        }
    }

    public String format(String text) 
    {
        if (text == null || "".equals(text))
        {
            return "";
        }

        int offset = 0;
        int literalsCount = 0;
        StringBuilder result = new StringBuilder();

        for (int i = 0; i < mask.length(); i++) 
        {
            final String patternKey = String.valueOf(mask.charAt(i));
            if (patternMap.containsKey(patternKey)) 
            {
                MaskPattern pattern = patternMap.get(patternKey); 
                final int nextIndex = indexOfFirstValidChar(pattern, text, offset);
                if (nextIndex >= 0 && nextIndex < text.length()) 
                {
                    offset = nextIndex + 1;

                    final char nextChar = text.charAt(nextIndex);
                    result.append(pattern.transform(nextChar));
                    literalsCount = 0;

                    if (offset >= text.length())
                    {
                        return result.toString();
                    }
                } 
                else 
                {
                    return result.substring(0, result.length() - literalsCount);
                } 
            } 
            else
            {
                result.append(mask.charAt(i));
                literalsCount++;
            }
        }

        return result.substring(0, result.length() - literalsCount);
    }

    private int indexOfFirstValidChar(MaskPattern pattern, String text, int offset) 
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
