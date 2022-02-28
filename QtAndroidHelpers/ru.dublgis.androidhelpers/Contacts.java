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

public class Contacts
{
	private static final String TAG = "Grym/Contacts";
	private Long mNativePtr = 0l;
	private static final String[] PROJECTION_PHONE = new String[]{
			ContactsContract.Contacts.DISPLAY_NAME,
			ContactsContract.CommonDataKinds.Phone.CONTACT_ID,
			ContactsContract.CommonDataKinds.Phone.TYPE,
			ContactsContract.CommonDataKinds.Phone.LABEL,
			ContactsContract.CommonDataKinds.Phone.NUMBER,
			ContactsContract.CommonDataKinds.Phone.NORMALIZED_NUMBER,
	};
	private static final String[] PROJECTION_EMAIL = new String[]{
			ContactsContract.Contacts.DISPLAY_NAME,
			ContactsContract.CommonDataKinds.Email.CONTACT_ID,
			ContactsContract.CommonDataKinds.Email.TYPE,
			ContactsContract.CommonDataKinds.Email.LABEL,
			ContactsContract.CommonDataKinds.Email.ADDRESS,
	};

	public static class Email
	{
		private final String mLabel;
		private final String mAddress;

		public Email(String label, String address)
		{
			mLabel = label;
			mAddress = address;
		}

		public String getLabel() { return mLabel; }
		public String getAddress() { return mAddress; }
	};

	public static class Phone
	{
		private final String mLabel;
		private final String mNumber;

		public Phone(String label, String number)
		{
			mLabel = label;
			mNumber = number;
		}

		public String getLabel() { return mLabel; }
		public String getNumber() { return mNumber; }
	};

	public static class Contact
	{
		private final String mId;
		private String mFullName;
		private List<Phone> mPhones = new ArrayList<Phone>();
		private List<Email> mEmails = new ArrayList<Email>();

		public Contact(String id) { mId = id; }

		public String getId() { return mId; }
		public String getFullName() { return mFullName; }
		public List<Phone> getPhones() { return mPhones; }
		public List<Email> getEmails() { return mEmails; }

		public void setFullName(String name) { mFullName = name; }
		public void addPhone(Phone phone) { mPhones.add(phone); }
		public void addEmail(Email email) { mEmails.add(email); }
	};

	public static class ContactContainer
	{
		private HashMap<String, Contact> mContacts = new HashMap<String, Contact>();

		public ArrayList<Contact> getContactList()
		{
			return new ArrayList<Contact>(mContacts.values());
		}

		public Contact getContact(String contactId)
		{
			if (!mContacts.containsKey(contactId)) {
				Contact contact = new Contact(contactId);
				mContacts.put(contactId, contact);
			}
			return mContacts.get(contactId);
		}
	};

	public Contacts(long nativePtr) { mNativePtr = nativePtr; }

	//! Called from C++ to notify us that the associated C++ object is being destroyed.
	public void cppDestroyed() {
		synchronized(mNativePtr) {
            mNativePtr = 0l;
        }
	}

	//! Called from C++
	public boolean checkPermission()
	{
		try {
			Context ctx = getContext();
			return ContextCompat.checkSelfPermission(ctx, Manifest.permission.READ_CONTACTS)
					== PackageManager.PERMISSION_GRANTED;
		} catch (final Throwable e) {
			Log.e(TAG, "checkPermission exception: ", e);
		}

		return false;
	}

	//! Called from C++
	public void requestContacts()
	{
		new Thread(new Runnable() {
			@Override public void run() {
				requestContactsInternal();
			}
		}).start();
	}

	private void requestContactsInternal()
	{
		Cursor cursor = null;
		Context ctx = null;
		ContactContainer contactContainer = new ContactContainer();

		try {
			ctx = getContext();
			ContentResolver cr = ctx.getContentResolver();

			cursor = cr.query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI,
					PROJECTION_PHONE,
					null,
					null,
					null);

			if (cursor != null) {
				final int contactIdIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.CONTACT_ID);
				final int contactNameIndex = cursor.getColumnIndex(ContactsContract.Contacts.DISPLAY_NAME);
				final int phoneTypeIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.TYPE);
				final int phoneLabelIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.LABEL);
				final int phoneNumberIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER);
				final int normPhoneNumberIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NORMALIZED_NUMBER);

				while (cursor.moveToNext()) {
					Contact contact = contactContainer.getContact(cursor.getString(contactIdIndex));
					contact.setFullName(cursor.getString(contactNameIndex));

					final CharSequence phoneLabel = ContactsContract.CommonDataKinds.Phone.getTypeLabel(
						ctx.getResources(),
						cursor.getInt(phoneTypeIndex),
						cursor.getString(phoneLabelIndex));

					final String phoneNumber = cursor.isNull(normPhoneNumberIndex)
						? cursor.getString(phoneNumberIndex)
						: cursor.getString(normPhoneNumberIndex);

					contact.addPhone(new Phone(phoneLabel.toString(), phoneNumber));
				}
				cursor.close();
			}

			cursor = cr.query(ContactsContract.CommonDataKinds.Email.CONTENT_URI,
					PROJECTION_EMAIL,
					null,
					null,
					null);

			if (cursor != null) {
				final int contactIdIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Email.CONTACT_ID);
				final int contactNameIndex = cursor.getColumnIndex(ContactsContract.Contacts.DISPLAY_NAME);
				final int emailTypeIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Email.TYPE);
				final int emailLabelIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Email.LABEL);
				final int emailAddressIndex = cursor.getColumnIndex(ContactsContract.CommonDataKinds.Email.ADDRESS);

				while (cursor.moveToNext()) {
					Contact contact = contactContainer.getContact(cursor.getString(contactIdIndex));
					contact.setFullName(cursor.getString(contactNameIndex));

					final CharSequence emailLabel = ContactsContract.CommonDataKinds.Email.getTypeLabel(
						ctx.getResources(),
						cursor.getInt(emailTypeIndex),
						cursor.getString(emailLabelIndex));

					contact.addEmail(new Email(
						emailLabel.toString(),
						cursor.getString(emailAddressIndex)));
				}
				cursor.close();
			}
		} catch(final Throwable e) {
			Log.e(TAG, "requestContactsInternal exception: ", e);
			cursor.close();
		}

		if (ctx != null) {
			ArrayList<Contact> contacts = contactContainer.getContactList();

			new Handler(ctx.getMainLooper()).post(new Runnable() {
				@Override public void run() {
					try {
						synchronized(mNativePtr) {
							nativeContactsReceived(mNativePtr, contacts);
						}
					} catch(final Throwable e) {
						Log.e(TAG, "call nativeContactsReceived exception: ", e);
					}
				}
			});
		}
	}

	public native Context getContext();
	public native void nativeContactsReceived(long nativePtr, java.lang.Object contactsList);
};
