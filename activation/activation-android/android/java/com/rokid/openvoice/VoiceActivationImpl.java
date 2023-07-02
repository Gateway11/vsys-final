package com.rokid.openvoice;

import java.util.List;
import java.util.ArrayList;
import android.util.Log;

class VoiceActivationImpl extends VoiceActivation {
	private static final String TAG = "VoiceActivationImpl";
	private static final int NATIVE_CONTROL_STATE_ACTIVE = 200;
	private static final int NATIVE_CONTROL_STATE_SLEEP = 201;
	private static final int NATIVE_VTWORD_TYPE_AWAKE = 1;
	private static final int NATIVE_VTWORD_TYPE_SLEEP = 2;
	// private static final int NATIVE_VTWORD_TYPE_HOT = 3;
	private static final int NATIVE_VOICE_EVENT_LOCAL_WAKE = 100;
	private static final int NATIVE_VOICE_EVENT_LOCAL_SLEEP = 101;
	private static final int NATIVE_VOICE_EVENT_VT_INFO = 102;
	private static final int NATIVE_VOICE_EVENT_WAKE_NOCMD = 104;
	private static final int NATIVE_VOICE_EVENT_VAD_COMING = 105;
	private static final int NATIVE_VOICE_EVENT_VAD_START = 106;
	private static final int NATIVE_VOICE_EVENT_VAD_DATA = 107;
	private static final int NATIVE_VOICE_EVENT_VAD_END = 108;
	private static final int NATIVE_VOICE_EVENT_VAD_CANCEL = 109;

	private long _handle;
	private VoiceActivation.Callback callback;

	public VoiceActivationImpl(long createParam, String basePath, boolean vadEnabled, VoiceActivation.Callback cb) {
		_handle = native_create_handle(createParam, basePath, vadEnabled, this);
		callback = cb;
	}

	public void finalize() {
		if (_handle != 0)
			native_free_handle(_handle);
	}

	public int process(byte[] input, int offset, int length) {
		if (_handle == 0)
			return -1;

		if (offset < 0 || offset + length > input.length) {
			Log.w(TAG, "process: input args invalid, input data array length = " + input.length + ", offset = " + offset + ", length = " + length);
			return -1;
		}
		return native_process(_handle, input, offset, length);
	}

	public int control(VoiceActivation.State state) {
		int in;

		if (_handle == 0)
			return -1;

		switch(state) {
			case ACTIVE:
				in = NATIVE_CONTROL_STATE_ACTIVE;
				break;
			case SLEEP:
				in = NATIVE_CONTROL_STATE_SLEEP;
				break;
			default:
				Log.e(TAG, "control: unknown state " + state);
				return -1;
		}
		return native_control(_handle, in);
	}

	public int addVtWord(VoiceActivation.VtWord word) {
		int tp;

		if (_handle == 0)
			return -1;

		switch (word.type) {
			case AWAKE:
				tp = NATIVE_VTWORD_TYPE_AWAKE;
				break;
			case SLEEP:
				tp = NATIVE_VTWORD_TYPE_SLEEP;
				break;
			// case HOT:
			//	tp = NATIVE_VTWORD_TYPE_HOT;
			//	break;
			default:
				Log.e(TAG, "addVtWord: unknown word type " + word.type);
				return -1;
		}
		return native_add_vt_word(_handle, tp, word.word, word.phone);
	}

	public int removeVtWord(String word) {
		if (_handle == 0)
			return -1;
		return native_remove_vt_word(_handle, word);
	}

	public List<VoiceActivation.VtWord> getVtWords() {
		int[] types = null;
		String[] words = null;
		String[] phones = null;
		int c;

		if (_handle == 0)
			return null;

		synchronized (this) {
			c = native_get_vt_words_count(_handle);
			if (c > 0) {
				types = new int[c];
				words = new String[c];
				phones = new String[c];
				native_fill_vt_words(_handle, types, words, phones);
			}
		}
		if (c == 0)
			return null;

		int i;
		VoiceActivation.VtWord vtw;
		ArrayList<VoiceActivation.VtWord> result = new ArrayList<VoiceActivation.VtWord>();
		for (i = 0; i < c; ++i) {
			vtw = new VoiceActivation.VtWord();
			vtw.type = vtwordType(types[i]);
			vtw.word = words[i];
			vtw.phone = phones[i];
			result.add(vtw);
		}
		return result;
	}

	private void nativeEventCallback(int event, int vtbegin, int vtend,
			float vtenergy, float energy, float energyThreshold, float location,
			byte[] data) {
		String word;

		switch (event) {
			case NATIVE_VOICE_EVENT_LOCAL_WAKE:
				Log.d(TAG, "callback onAwake");
				callback.onAwake();
				break;
			case NATIVE_VOICE_EVENT_LOCAL_SLEEP:
				Log.d(TAG, "callback onSleep");
				callback.onSleep();
				break;
			case NATIVE_VOICE_EVENT_VT_INFO:
				if (data != null) {
					word = new String(data);
					Log.d(TAG, "callback onVoiceTrigger " + word + ", " + vtbegin + " -- " + vtend + ", " + vtenergy);
					callback.onVoiceTrigger(word, vtbegin, vtend, vtenergy);
				}
				break;
			case NATIVE_VOICE_EVENT_WAKE_NOCMD:
				Log.d(TAG, "callback onAwakeNoCmd");
				callback.onAwakeNoCmd();
				break;
			case NATIVE_VOICE_EVENT_VAD_COMING:
				Log.d(TAG, "callback onVadComing " + location);
				callback.onVadComing(location);
				break;
			case NATIVE_VOICE_EVENT_VAD_START:
				Log.d(TAG, "callback onVadStart " + energy + ", " + energyThreshold);
				callback.onVadStart(energy, energyThreshold);
				break;
			case NATIVE_VOICE_EVENT_VAD_DATA:
				Log.d(TAG, "callback onVadData " + data.length);
				callback.onVadData(data);
				break;
			case NATIVE_VOICE_EVENT_VAD_END:
				Log.d(TAG, "callback onVadEnd");
				callback.onVadEnd();
				break;
			case NATIVE_VOICE_EVENT_VAD_CANCEL:
				Log.d(TAG, "callback onVadCancel");
				callback.onVadCancel();
				break;
		}
	}

	private native long native_create_handle(long param, String basePath, boolean vadEnabled, VoiceActivationImpl inst);

	private native void native_free_handle(long handle);

	private native int native_process(long handle, byte[] input, int offset, int length);

	private native int native_control(long handle, int st);

	private native int native_add_vt_word(long handle, int tp, String w, String p);

	private native int native_remove_vt_word(long handle, String w);

	private native int native_get_vt_words_count(long handle);

	private native void native_fill_vt_words(long handle, int[] types, String[] words, String[] phones);

	private static VoiceActivation.VtWord.Type vtwordType(int tp) {
		switch (tp) {
			case NATIVE_VTWORD_TYPE_SLEEP:
				return VoiceActivation.VtWord.Type.SLEEP;
			// case NATIVE_VTWORD_TYPE_HOT:
			//	return HOT;
		}
		return VoiceActivation.VtWord.Type.AWAKE;
	}
}
