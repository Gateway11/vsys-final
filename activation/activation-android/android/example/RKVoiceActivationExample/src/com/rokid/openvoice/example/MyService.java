package com.rokid.openvoice.example;

import java.io.File;
import java.io.FileOutputStream;
import java.io.InputStream;
import java.util.List;

import android.app.Service;
import android.content.Intent;
import android.content.res.AssetManager;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Environment;
import android.os.IBinder;
import android.util.Log;

import com.rokid.openvoice.VoiceActivation;
import com.rokid.openvoice.VoiceActivationBuilder;
import com.rokid.openvoice.VoiceActivation.State;
import com.rokid.openvoice.VoiceActivation.VtWord;

public class MyService extends Service implements Runnable,
		VoiceActivation.Callback {
	public MyService() {
	}

	@Override
	public IBinder onBind(Intent intent) {
		// TODO: Return the communication channel to the service.
		throw new UnsupportedOperationException("Not yet implemented");
	}

	
	@Override
	public void onCreate() {
		new Thread(this).start();
	}

	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		return START_NOT_STICKY;
	}

	@Override
	public void run() {
		if (!prepareWorkDirAndFiles()) {
			Log.e(TAG, "create workdir_cn and files failed");
			return;
		}

		VoiceActivationBuilder builder = new VoiceActivationBuilder();
		// 设置输入pcm流的采样率，位宽
		// 麦克风数
		// 本地vad模式
		// 设备上workdir_cn所在目录(算法模块需要读取此目录下模型文件)
		// 单通道，通道高级参数不设置，全部忽略
		// 回调对象
		VoiceActivation va = builder
				.setSampleRate(16000)
				.setBitsPerSample(VoiceActivationBuilder.AudioFormat.ENCODING_PCM_16BIT)
				.setChannelNumber(1)
				.enableVad(true)
				.setBasePath(Environment.getExternalStorageDirectory().getPath())
				.addMic(0, 0, 0, 0, 0).setMicParamMask(0)
				.setCallback(this).build();
		
//		VoiceActivation.VtWord mVtWord = new VoiceActivation.VtWord();
//		mVtWord.type = VoiceActivation.VtWord.Type.AWAKE;
//		mVtWord.word = "大傻逼";
//		mVtWord.phone = "da4sha3bi1";
//		va.addVtWord(mVtWord);
		
		processVoiceData(va);
	}

	private boolean prepareWorkDirAndFiles() {
		AssetManager am = getApplicationContext().getAssets();
		try {
			copyAssetDir(am, "workdir_cn", Environment
					.getExternalStorageDirectory().getPath() + "/workdir_cn");
			// pcmRecFile = new
			// FileOutputStream(Environment.getExternalStorageDirectory().getPath()
			// + "/rkpcmrec");
		} catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}

	private boolean copyAssetDir(AssetManager am, String src, String dst)
			throws Exception {
		File f = new File(dst);
		if (!f.exists() && !f.mkdir()) {
			Log.e(TAG, "mkdir " + dst + " failed");
			return false;
		}
		String[] files = am.list(src);
		String[] subfiles;
		int i;
		int c;
		InputStream is;
		FileOutputStream os;
		byte[] buf = new byte[2048];
		for (i = 0; i < files.length; ++i) {
			subfiles = am.list(src + "/" + files[i]);
			if (subfiles.length > 0) {
				if (!copyAssetDir(am, src + "/" + files[i], dst + "/"
						+ files[i]))
					return false;
			} else {
				is = am.open(src + "/" + files[i]);
				os = new FileOutputStream(dst + "/" + files[i]);
				while (true) {
					c = is.read(buf);
					if (c <= 0)
						break;
					os.write(buf, 0, c);
				}
			}
		}
		return true;
	}

	private void processVoiceData(VoiceActivation va) {
		AudioRecord ar;
		int min;
		min = AudioRecord.getMinBufferSize(16000, 16, 2);
		ar = new AudioRecord(MediaRecorder.AudioSource.VOICE_COMMUNICATION,
				16000, 16, 2, min * 5);

		byte[] buf = new byte[min];
		int c;
		
		ar.startRecording();
		while (true) {
			c = ar.read(buf, 0, buf.length);
			if (c > 0) {
				// try {
				// pcmRecFile.write(buf, 0, c);
				// } catch (Exception e) {
				// e.printStackTrace();
				// }
				va.process(buf, 0, c);
			}
		}
		// ar.stop();
	}

	@Override
	public void onAwake() {
		Log.d(TAG, "onAwake");
	}

	@Override
	public void onAwakeNoCmd() {
		Log.d(TAG, "onAwakeNoCmd");
	}

	@Override
	public void onSleep() {
		Log.d(TAG, "onSleep");
	}

	@Override
	public void onVadCancel() {
		Log.d(TAG, "onVadCancel");
	}

	@Override
	public void onVadComing(float arg0) {
		Log.d(TAG, "onVadComing: location = " + arg0);
	}

	@Override
	public void onVadData(byte[] arg0) {
		Log.d(TAG, "onVadData: data is " + arg0.length + " bytes");
	}

	@Override
	public void onVadEnd() {
		Log.d(TAG, "onVadEnd");
	}

	@Override
	public void onVadStart(float arg0, float arg1) {
		Log.d(TAG, "onVadStart: energy = " + arg0 + ", energy threshold = "
				+ arg1);
	}

	@Override
	public void onVoiceTrigger(String arg0, int arg1, int arg2, float arg3) {
		Log.d(TAG, "onVoiceTrigger: word = " + arg0 + ", start = " + arg1
				+ ", end = " + arg2 + ", energy = " + arg3);
	}

	// private FileOutputStream pcmRecFile;

	// private static final String VOICE_FILES[] = {
	// "rkpcm1",
	// "rkpcm2",
	// "rkpcm3",
	// "rkpcm4",
	// };
	// private static final String RKWORK_FILES[] = {
	// "workdir_cn/final.ruoqi.mod",
	// "workdir_cn/final.svd.mod",
	// "workdir_cn/phonetable"
	// };
	// private static final int PCM_FRAME_SIZE = 320;
	private static final String TAG = "VoiceActivation.example";
}
