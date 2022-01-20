/*
	Offscreen Android Views library for Qt

	Author:
	Aleksey I. Gribanov <a.gribanov@2gis.ru>

	Distrbuted under The BSD License

	Copyright (c) 2022, DoubleGIS, LLC.
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

import android.Manifest;
import android.app.Activity;
import android.content.ContentResolver;
import android.content.pm.PackageManager;
import android.os.Looper;
import android.os.Handler;
import android.provider.ContactsContract;
import android.provider.Settings;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;

import androidx.core.content.ContextCompat;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.lang.Object;

import ru.dublgis.androidhelpers.Log;

class Contact
{
	public String mFullName;
	public List<String> mPhones = new ArrayList<String>();
	public List<String> mEmails = new ArrayList<String>();
};

class ContactsHash
{
	private HashMap<String, Contact> mContacts = new HashMap<String, Contact>();

	public ArrayList<Contact> getContactList()
	{
		return new ArrayList<Contact>(mContacts.values());
	}

	public Contact getContact(String contactId)
	{
		if (!mContacts.containsKey(contactId)) {
			Contact contact = new Contact();
			mContacts.put(contactId, contact);
		}
		return mContacts.get(contactId);
	}
};

public class Contacts
{
	private static final String TAG = "Grym/Contacts";
	private long mNativePtr = 0;
	private static final String[] PROJECTION_PHONE = new String[]{
			ContactsContract.CommonDataKinds.Phone.CONTACT_ID,
			ContactsContract.Contacts.DISPLAY_NAME,
			ContactsContract.CommonDataKinds.Phone.NUMBER,
	};
	private static final String[] PROJECTION_EMAIL = new String[]{
			ContactsContract.CommonDataKinds.Email.CONTACT_ID,
			ContactsContract.CommonDataKinds.Email.ADDRESS,
			ContactsContract.CommonDataKinds.Email.DISPLAY_NAME
	};

	public Contacts(long native_ptr) { mNativePtr = native_ptr; }

	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed() {  }

	public boolean checkPermission()
	{
		Context ctx = getContext();
		return ContextCompat.checkSelfPermission(ctx, Manifest.permission.READ_CONTACTS)
				== PackageManager.PERMISSION_GRANTED;
	}

	public void readContacts()
	{
		new Thread(new Runnable() {
			@Override public void run() { getContactList(); }
		}).start();
	}

	private void getContactList()
	{
		Context ctx = getContext();
		ContentResolver cr = ctx.getContentResolver();

		ContactsHash contactsHash = new ContactsHash();
		Cursor cursor = cr.query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI,
				PROJECTION_PHONE,
				null,
				null,
				null);

		if (cursor != null) {
			try {
				final int idIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.CONTACT_ID);
				final int nameIndex = cursor.getColumnIndex(ContactsContract.Contacts.DISPLAY_NAME);
				final int numberIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER);
				while (cursor.moveToNext()) {
					Contact contact = contactsHash.getContact(cursor.getString(idIndex));
					contact.mFullName = cursor.getString(nameIndex);
					contact.mPhones.add(cursor.getString(numberIndex));
				}
			} finally {
				cursor.close();
			}
		}

		Cursor cursorEmail = cr.query(ContactsContract.CommonDataKinds.Email.CONTENT_URI,
				PROJECTION_EMAIL,
				null,
				null, null);

		if (cursorEmail != null) {
			try {
				final int idIndex = cursorEmail.getColumnIndex(ContactsContract.CommonDataKinds.Email.CONTACT_ID);
				final int emailIndex = cursorEmail.getColumnIndex(ContactsContract.CommonDataKinds.Email.ADDRESS);
				while (cursorEmail.moveToNext()) {
					Contact contact = contactsHash.getContact(cursorEmail.getString(idIndex));
					contact.mEmails.add(cursorEmail.getString(emailIndex));
				}
			} finally {
				cursorEmail.close();
			}
		}

		ArrayList<Contact> contacts = contactsHash.getContactList();
		new Handler(ctx.getMainLooper()).post(new Runnable() {
			@Override public void run() {
				nativeRecievedContacts(mNativePtr, contacts);
			}
		});
	}

	public native Context getContext();
	public native void nativeRecievedContacts(long nativeptr, java.lang.Object contactsList);
};
