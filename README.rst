********************
nginx-vrf-ext-module
********************

NGINX support for the $request_vrf variable containing the incoming
request's VRF name, based on the interface an incoming connection's
socket is bound to.  This is only supported on Linux kernels with VRF
support and sysctl variable net.ipv4.tcp_l3mdev_accept set to 1.

| Copyright |copy| 2018 Green Communications (qolyester@green-communications.fr)

.. |copy|   unicode:: U+000A9 .. COPYRIGHT SIGN

Example config
==============

.. code-block::

	location / {
		if ($request_vrf = "vlan-42") {
			return 405;
		}

		...
	}
