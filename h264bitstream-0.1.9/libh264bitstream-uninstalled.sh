# generated by configure / remove this line to disable regeneration
prefix="/usr/local"
exec_prefix="${prefix}"
bindir="${exec_prefix}/bin"
libdir="/home/cruxeon/Desktop/Streaming/h264bitstream-0.1.9"
datarootdir="${prefix}/share"
datadir="${datarootdir}"
sysconfdir="${prefix}/etc"
includedir="/home/cruxeon/Desktop/Streaming/h264bitstream-0.1.9/."
package="h264bitstream"
suffix=""

for option; do case "$option" in --list-all|--name) echo  "libh264bitstream"
;; --help) pkg-config --help ; echo Buildscript Of "libh264bitstream Library"
;; --modversion|--version) echo "0.1.9"
;; --requires) echo : ""
;; --libs) echo -L${libdir} "" "${libdir}/libh264bitstream.la"
       :
;; --cflags) echo -I${includedir} ""
       :
;; --variable=*) eval echo '$'`echo $option | sed -e 's/.*=//'`
;; --uninstalled) exit 0 
;; *) ;; esac done
