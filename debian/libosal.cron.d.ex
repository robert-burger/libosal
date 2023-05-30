#
# Regular cron jobs for the libosal package
#
0 4	* * *	root	[ -x /usr/bin/libosal_maintenance ] && /usr/bin/libosal_maintenance
