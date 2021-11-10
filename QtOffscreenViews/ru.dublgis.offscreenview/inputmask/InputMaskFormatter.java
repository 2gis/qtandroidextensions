/*
  InputMask for Offscreen Android Views library for Qt

  Author:
  Yuriy Y Ibetullov <mrlazgen@gmail.com>

  Distrbuted under The BSD License

  Copyright (c) 2021, DoubleGIS, LLC.
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the DoubleGIS, LLC nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
  THE POSSIBILITY OF SUCH DAMAGE.
*/

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
    
    private ValidationState state_ = ValidationState.Invalid;
    private HashMap<String, MaskPattern> maskPatterns_;
    private String lastText_;

    enum ValidationState 
    {
        Invalid,
        Intermediate,
        Acceptable
    }

    public class MaskPattern 
    {
        private Pattern pattern_;

        public MaskPattern(String regex) 
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

    public InputMaskFormatter(String mask) 
    {
        initPatterns();
        setMask(mask);
    }

    public void setMask(String mask) 
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

    private void parseMask(String mask) 
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

    public String format(String text) 
    {
        if (text == null)
        {
            state_ = ValidationState.Invalid;
            return "";
        }

        int offset = 0;
        StringBuilder result;
        if (lastText_ != null && text.startsWith(lastText_)) 
        {
            offset = lastText_.length();
            result = new StringBuilder(lastText_);
        }
        else
        {
            result = new StringBuilder();
        }

        // Формируем строку по маске
        if (!text.isEmpty()) 
        {
            for (int i = offset; i < mask_.length(); i++) 
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

                        // Дошли до конца строки, больше нет символов
                        if (offset >= text.length())
                        {   
                            offset = i;
                            break;
                        }
                    } 
                    else 
                    {
                        // Больше нет валидных символов
                        offset = i;
                        break;
                    }
                } 
                else
                {
                    result.append(mask_.charAt(i));
                }
            }
        }

        lastText_ = result.toString();

        if (lastText_.length() == mask_.length()) 
        {
            state_ = ValidationState.Acceptable;
        } 
        else
        {
            // Проверяем заверщенность строки
            for (int j = offset; j < mask_.length(); j++) 
            {
                final String key = String.valueOf(mask_.charAt(j));
                final MaskPattern p = maskPatterns_.get(key);
                if (p != null && p.isValid("")) 
                {
                    state_ = ValidationState.Acceptable;
                } 
                else
                {
                    state_ = ValidationState.Intermediate;
                    break;
                }
            }
        }

        return lastText_;
    }

    public ValidationState state()
    {
        return state_;
    }

    //! return true if inputMask is set and text is acceptable as a final result else return false
    public boolean hasAcceptableInput()
    {
        return state_ == ValidationState.Acceptable;
    }
    
    public String mask()
    {
        return mask_;
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
