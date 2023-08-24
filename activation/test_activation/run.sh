##!/bin/bash
#!/vendor/bin/sh

export LD_LIBRARY_PATH=/data/vsys-final:$LD_LIBRARY_PATH

chmod 777 test_activation

./test_activation
