package com.rokid.openvoice;

import java.util.ArrayList;
import java.util.Iterator;
import android.util.Log;

public class VoiceActivationBuilder {

	private static final String TAG = "VoiceActivationBuilder";

	public static final int MASK_MIC_POSITION           = 0x1 << 0;
	public static final int MASK_MIC_DELAY              = MASK_MIC_POSITION | 0x1 << 1;

	private int sampleRate;
	private int bitsPerSample;
	private int channelNumber;
	private int micParamMask;
	private boolean vadEnabled;
	private String basePath;
	private VoiceActivation.Callback callback;
	private ArrayList<Mic> mics;

	public VoiceActivationBuilder() {
		sampleRate = 0;
		bitsPerSample = 0;
		channelNumber = 0;
		micParamMask = 0;
		vadEnabled = false;
		mics = new ArrayList<Mic>();
	}

	public VoiceActivationBuilder setSampleRate(int sampleRate) {
		this.sampleRate = sampleRate;
		return this;
	}

	public VoiceActivationBuilder setBitsPerSample(int bits) {
		bitsPerSample = bits;
		return this;
	}

	public VoiceActivationBuilder setChannelNumber(int num) {
		channelNumber = num;
		return this;
	}

	public VoiceActivationBuilder enableVad(boolean b) {
		vadEnabled = b;
		return this;
	}

	// MASK_MIC_POSITION
	// MASK_MIC_DELAY
	public VoiceActivationBuilder setMicParamMask(int mask) {
		micParamMask = mask;
		return this;
	}

	// 如果ChannelNumber为n
	// 'idx' 范围为0~n
	// 如果micParamMask设置了MASK_MIC_POSITION位, posx, posy, posz将被作为有效参数处理
	// 如果micParamMask设置了MASK_MIC_DELAY位, delay将被作为有效参数处理
	// 如果多次调用addMic，传入了重复的'idx'，将只有第一次传入该'idx'的addMic生效
	public VoiceActivationBuilder addMic(int idx, float posx, float posy, float posz, float delay) {
		Mic mic = new Mic();
		mic.idx = idx;
		mic.posx = posx;
		mic.posy = posy;
		mic.posz = posz;
		mic.delay = delay;
		mics.add(mic);
		return this;
	}

	// 设置workdir_cn目录的父目录位置
	public VoiceActivationBuilder setBasePath(String path) {
		basePath = path;
		return this;
	}

	public VoiceActivationBuilder setCallback(VoiceActivation.Callback cb) {
		callback = cb;
		return this;
	}

	public VoiceActivation build() {
		if (!ParamChecker.isValidSampleRate(sampleRate))
			return null;
		if (!ParamChecker.isValidBitsPerSample(bitsPerSample))
			return null;
		if (channelNumber <= 0) {
			Log.e(TAG, "invalid channelNumber " + channelNumber);
			return null;
		}
		if (basePath == null) {
			Log.e(TAG, "base path not set");
			return null;
		}
		if (callback == null) {
			Log.e(TAG, "callback not set");
			return null;
		}
		mics = ParamChecker.checkMicParams(channelNumber, mics);
		CreateArgs createArgs = new CreateArgs(sampleRate, bitsPerSample, channelNumber, micParamMask, mics);
		Log.d(TAG, "VoiceActivation instance build:");
		Log.d(TAG, "sample rate: " + sampleRate);
		Log.d(TAG, "bits per sample: " + bitsPerSample);
		Log.d(TAG, "mic number: " + mics.size());
		Log.d(TAG, mics.toString());
		Log.d(TAG, "channel number: " + channelNumber);
		Log.d(TAG, "base path: " + basePath);
		return new VoiceActivationImpl(createArgs.nativeParam(), basePath, vadEnabled, callback);
	}

	private static class Mic {
		public int idx;
		public float posx;
		public float posy;
		public float posz;
		public float delay;

		public Mic() {
			idx = 0;
			posx = 0;
			posy = 0;
			posz = 0;
			delay = 0;
		}
	}

	private static class CreateArgs {
		public CreateArgs(int sampleRate, int bitsPerSample, int channelNumber, int micParamMask, ArrayList<Mic> mics) {
			Iterator<Mic> it;
			Mic mic;

			_param = native_create_param(sampleRate, bitsPerSample, channelNumber, micParamMask);
			it = mics.iterator();
			while (it.hasNext()) {
				mic = it.next();
				native_add_mic(_param, mic.idx, mic.posx, mic.posy, mic.posz, mic.delay);
			}
		}

		public long nativeParam() {
			return _param;
		}

		private native long native_create_param(int sampleRate, int bits, int micN, int mask);

		private native void native_add_mic(long h, int idx, float x, float y, float z, float delay);

		private long _param;
	}

	public static class AudioFormat{
		public static final int ENCODING_PCM_16BIT      = 0x1;
		public static final int ENCODING_PCM_24BIT      = 0x2;
		public static final int ENCODING_PCM_32BIT      = 0x3;
		public static final int ENCODING_PCM_FLOAT      = 0x4;
	}

	private static class ParamChecker {
		private static final int[] SUPPORTED_SAMPLE_RATE = { 16000 };
		private static final String[] SUPPORTED_BITS_PER_SAMPLE_STRING = {
			"ENCODING_PCM_16BIT", 
			"ENCODING_PCM_24BIT", 
			"ENCODING_PCM_32BIT", 
			"ENCODING_PCM_FLOAT"
		};

		public static boolean isValidSampleRate(int sampleRate) {
			int i;
			for (i = 0; i < SUPPORTED_SAMPLE_RATE.length; ++i) {
				if (sampleRate == SUPPORTED_SAMPLE_RATE[i])
					return true;
			}
			Log.e(TAG, "unsupport sample rate " + sampleRate + ".\nsupported sample rate: " + SUPPORTED_SAMPLE_RATE);
			return false;
		}

		public static boolean isValidBitsPerSample(int bits) {
			switch(bits){
				case AudioFormat.ENCODING_PCM_16BIT:
				case AudioFormat.ENCODING_PCM_24BIT:
				case AudioFormat.ENCODING_PCM_32BIT:
				case AudioFormat.ENCODING_PCM_FLOAT:
					return true;
			}
			Log.e(TAG, "unsupport bits per sample " + bits + ".\nsupported bits per sample: " + SUPPORTED_BITS_PER_SAMPLE_STRING);
			return false;
		}

		public static ArrayList<Mic> checkMicParams(int channelNumber, ArrayList<Mic> mics) {
			Iterator<Mic> it;
			ArrayList<Mic> result = new ArrayList<Mic>();
			Mic mic;
			int i;
			boolean dup;

			it = mics.iterator();
			while (it.hasNext()) {
				mic = it.next();
				if (mic.idx < 0 || mic.idx >= channelNumber) {
					Log.w(TAG, "invalid mic index " + mic.idx + ", expected index range [0, " + (channelNumber - 1) + "].");
					Log.w(TAG, "ignore the mic");
					continue;
				}

				dup = false;
				for (i = 0; i < result.size(); ++i) {
					if (result.get(i).idx == mic.idx) {
						dup = true;
						Log.w(TAG, "mic index " + mic.idx + " already exists, ignore the mic");
						break;
					}
				}
				if (dup)
					continue;

				result.add(mic);
			}
			Log.d(TAG, "mic number is " + channelNumber + ", mic number is " + result.size());
			return result;
		}
	}

	static {
		System.loadLibrary("rkvacti_jni");
	}
}
