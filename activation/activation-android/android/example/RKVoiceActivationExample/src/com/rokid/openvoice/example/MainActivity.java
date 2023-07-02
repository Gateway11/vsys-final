package com.rokid.openvoice.example;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Intent;
import android.os.Bundle;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		Intent intent = new Intent();
		intent.setComponent(new ComponentName("com.rokid.openvoice.example", "com.rokid.openvoice.example.MyService"));
		this.startService(intent);
	}
}
