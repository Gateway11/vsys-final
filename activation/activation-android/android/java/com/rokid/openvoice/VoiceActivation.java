package com.rokid.openvoice;

import java.util.List;

public abstract class VoiceActivation {
	public int process(byte[] input) {
		return process(input, 0, input.length);
	}

	public abstract int process(byte[] input, int offset, int length);

	public abstract int control(State state);

	public abstract int addVtWord(VtWord word);

	public abstract int removeVtWord(String word);

	public abstract List<VtWord> getVtWords();

	public static enum State {
		// 处理输入音频数据流并回调音频数据流
		// 输入音频数据中发现休眠词或外部调用接口control(SLEEP)，状态变为SLEEP
		ACTIVE,
		// 处理输入音频数据流但不回调
		// 输入音频数据流中发现唤醒词或外部调用接口control(ACTIVE), 状态变为ACTIVE
		SLEEP
	}

	public static class VtWord {
		public static enum Type {
			// 唤醒词
			AWAKE,
			// 休眠词
			SLEEP
		}

		public Type type;
		public String word; // utf8编码
		public String phone;
	}

	public static interface Callback {
		public void onAwake();
		public void onSleep();
		public void onVoiceTrigger(String word, int begin, int end, float energy);
		public void onAwakeNoCmd();
		public void onVadComing(float location);
		public void onVadStart(float energy, float energyThreshold);
		public void onVadData(byte[] data);
		public void onVadEnd();
		public void onVadCancel();
	}
}
