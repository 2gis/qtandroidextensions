/*
    Offscreen Android Views library for Qt

    Author:
    Timur N. Artikov <t.artikov@gmail.com>

    Distrbuted under The BSD License

    Copyright (c) 2020, DoubleGIS, LLC.
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

package ru.dublgis.androidhelpers;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

class Zip {
    static void compressFiles(final String[] srcPaths, final String dstPath) throws IOException {
        try (final ZipOutputStream outputStream = new ZipOutputStream(new FileOutputStream(dstPath))) {
            final byte[] buffer = new byte[4096];
            for (final String srcPath : srcPaths) {
                final File file = new File(srcPath);
                if (!file.exists()) {
                    throw new FileNotFoundException(srcPath);
                }
                processFile(file.getParent(), file.getName(), outputStream, buffer);
            }
        }
    }

    private static void processFile(final String root, final String path,
                                    final ZipOutputStream outputStream, final byte[] buffer) throws IOException {
        final File file = new File(root + File.separator + path);
        if (file.isFile()) {
            try (FileInputStream inputStream = new FileInputStream(file)) {
                final ZipEntry entry = new ZipEntry(path);
                outputStream.putNextEntry(entry);
                int len;
                while ((len = inputStream.read(buffer)) != -1) {
                    outputStream.write(buffer, 0, len);
                }
                outputStream.closeEntry();
            }
        } else if (file.isDirectory()) {
            final String[] files = file.list();
            if (files == null || files.length == 0) {
                final ZipEntry entry = new ZipEntry(path + File.separator);
                outputStream.putNextEntry(entry);
                outputStream.closeEntry();
            } else {
                for (final String f : files) {
                    processFile(root, path + File.separator + f, outputStream, buffer);
                }
            }
        }
    }
}
