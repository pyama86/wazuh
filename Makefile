docker:
	docker build -t wazuh_develop .
	docker run -v `pwd`/src:/opt/wazuh -w /opt/wazuh -it wazuh_develop /bin/bash
