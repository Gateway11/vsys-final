
//<uses-permission android:name="android.permission.CAMERA"/>
public class CameraActivity extends AppCompatActivity {
    private SurfaceView surfaceView;
    private CameraContextNative cameraContext; // 保存native上下文指针

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_camera);
        surfaceView = findViewById(R.id.surface_view);
        surfaceView.getHolder().addCallback(new SurfaceHolder.Callback() {
            @Override
            public void surfaceCreated(SurfaceHolder holder) {
                // JNI调用启动相机
                cameraContext = startCameraNative(holder.getSurface());
            }

            @Override
            public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {}

            @Override
            public void surfaceDestroyed(SurfaceHolder holder) {
                // JNI调用停止相机
                stopCameraNative(cameraContext);
            }
        });
    }

    // Native方法声明
    public native long startCameraNative(Surface surface);
    public native void stopCameraNative(long contextPtr);
}
