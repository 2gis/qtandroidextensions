package ru.dublgis.offscreenview.inputmask;

import android.text.Editable;
import android.text.TextWatcher;
import android.widget.EditText;
import android.widget.TextView;

import java.util.HashMap;
import java.util.regex.Pattern;

import ru.dublgis.androidhelpers.Log;
import ru.dublgis.offscreenview.inputmask.InputMaskFormatter;

public class InputMaskTextWatcher implements TextWatcher 
{
    
    protected InputMaskFormatter formatter;
    protected EditText editText;
    protected String currentText;

    public InputMaskTextWatcher(EditText editText, InputMaskFormatter formatter)
     {
        this.editText = editText;
        this.formatter = formatter;
    }

    @Override
    public void beforeTextChanged(CharSequence s, int start, int count, int after) 
    {
    }

    @Override
    public void onTextChanged(CharSequence s, int start, int count, int after) 
    {
        final String text = s.toString();
        if (!text.equals(currentText)) 
        {
            try 
            {
                currentText = formatter.format(text);
            } 
            catch (final Throwable e) 
            {
                Log.w("InputMaskTextWatcher format text: '" + text + "' exception: ", e);
            }
            editText.setText(currentText);
            editText.setSelection(currentText.length());
        }
    }

    @Override
    public void afterTextChanged(Editable editable) 
    {
    }

}