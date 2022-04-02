
#!/bin/bash
set -e

echo "Compiling source ..."
g++ main.cpp -o obj-to-ivx

echo "Moving binary to /bin ..."
sudo mv obj-to-ivx /bin/obj-to-ivx

exit 0
