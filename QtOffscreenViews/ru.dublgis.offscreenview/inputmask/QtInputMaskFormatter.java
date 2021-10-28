package ru.dublgis.offscreenview.inputmask;

public class QtInputMaskFormatter extends InputMaskFormatter 
{
    // Some analog of https://doc.qt.io/qt-5/qlineedit.html#inputMask-prop 
    public QtInputMaskFormatter(String mask) 
    {
        super(mask);

        registerPattern("A", "\\p{Alpha}");
        registerPattern("a", "\\p{Alpha}?");
        registerPattern("N", "\\p{Alnum}");
        registerPattern("n", "\\p{Alnum}?");
        registerPattern("X", "\\p{Blank}");
        registerPattern("x", "\\p{Blank}?");
        registerPattern("9", "\\p{Digit}");
        registerPattern("0", "\\p{Digit}?");
        registerPattern("D", "[1-9]");
        registerPattern("d", "[1-9]?");
        registerPattern("H", "\\p{XDigit}");
        registerPattern("h", "\\p{XDigit}?");
    }
}
