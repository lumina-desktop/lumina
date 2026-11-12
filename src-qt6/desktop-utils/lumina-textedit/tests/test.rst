.. comment grays out
   continues with same spacing
breaks when justify left
sample2
.. TODO Black highlighting and brown/orange text for visibility on "TODO" with the rest greyed out like a comment.

TODO
      
.. code-block:: JSON
   Everything in a code block is blue text
   
.. testlink: will be blue text
.. _testlink: 

`github repository <https://github.com/pcbsd/sysadm.git>`_ from ` to _ is green text for hyperlinks

.. figures/images ".. figure::" is blue text, image name is standard black text
   any additional options for the figure "scale:" is purple text, value is black text.
   
directives :command:`test` from "`" to "`" is yellow text.

.. figure:: images/lumina1e.png
   :scale: 50%

::



.. code-block:: python

 

.. _gettingstarted:

Getting Started
===============

Beginning with SysAdm™ is a relatively simple process. 
SysAdm™ files are currently available from the `github repository <https://github.com/pcbsd/sysadm.git>`_


.. _building:

Building SysAdm™
----------------
sample
The following Qt Modules are required before attempting to build 
SysAdm™: ::

  Qt5 Core (# pkg install qt5-core)
  Qt5 Concurrent (# pkg install qt5-concurrent)
  Qt5 Network (# pkg install qt-network)
  Qt5 Sql (# pkg install qt5-sql)
  Qt5 Websockets (# pkg install qt5-websockets)

Building the prototype version of SysAdm™ assumes you have access to 
github.com. ::

  % git clone https://github.com/pcbsd/sysadm.git
  % cd sysadm/src
  % /usr/local/lib/qt5/bin/qmake -recursive
  % make && sudo make install

.. _starting:

Starting SysAdm™
----------------

SysAdm can be started one of two ways: 1. The traditional rc(8) 
mechanism 2. The new jobd(8) mechanism

To run under rc(8)::

 (For WebSockets - Required for SysAdm Client)
 % sudo sysrc -f /etc/rc.conf sysadm_enable="YES"
 % sudo service sysadm start

 (Optional for REST)
 % sudo sysrc -f /etc/rc.conf sysadm_rest_enable="YES"
 % sudo service sysadm-rest start


To run under jobd(8)::

 (For WebSockets - Required for SysAdm Client)
 % sudo jobctl org.pcbsd.sysadm enable

 (Optional for REST)
 % sudo jobctl org.pcbsd.sysadm-rest enable

.. _bridge init:

Bridge Initialization
---------------------

Configuring and connecting to a bridge can be a complicated process. 
Thankfully, there are several steps that are done the first time a 
server and bridge are configured with SysAdm but do not need to be 
repeated later. Once these steps are complete, it becomes a much simpler
process for a new user to configure their client to communicate with the
now configured server and bridge.

.. note:: A list of current commands is available by typing :command:`-h`
          after the utility name (Example: :command:`sysadm-bridge -h`).

.. _serverbridge init:

Server and Bridge Initialization
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To initialize the server and bridge, begin with the server. Run 
:command:`sudo sysadm-binary bridge_export_key [optional absolute file path]`.
This will export the public SSL key the server uses to authenticate with
the bridge.

.. note:: For both server and client, giving SSL key files an easy to 
          remember name and location will simplify the process of 
          finding those files for import to the bridge.

Now, we must transition to the bridge to import the server key. Login to
the bridge as the administrator (or root), then type 
:command:`sysadm-bridge import_ssl_file <filename> <filepath>`, 
replacing <filename> and <filepath> with the server key filename and 
location. Once the server key file is successfully imported, start the 
bridge (if not already running).

.. note:: The bridge can import SSL files whether it is active or not 
          with no negative effects.

Back on the server, run :command:`sudo sysadm-binary bridge_add <nickname> <URL>`
to point the server at the bridge. A bridge runs on **port 12149** by 
default, so the URL will likely need **:12149** added on the end of the 
address (Example URL: 127.0.0.1:12149). If necessary, (re)start the 
server. The log (:file:`/var/log/sysadm-server-ws.log`) will display 
messages about connecting to the bridge. If properly configured, the 
server and bridge will now be communicating with each other. At this 
point clients can be added to the mix which will communicate with the 
server through the bridge.

.. _add client:

Adding a Client to the Server/Bridge Connection
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note:: If you have an old SSL bundle from a pre-alpha version of 
          SysAdm created before June 2016, it will need to be removed 
          prior to proceeding with the client initialization process.

In the client UI, create or import an SSL key bundle as prompted by the 
UI. Once the new SSL keys are created, open 
:menuselection:`Setup SSL --> View Certificates` in the connection 
manager and click "Export Public Key" for both the server and bridge 
keys. This will export both SSL keys in file form, depositing them in 
either the "Desktop" folder or home directory (depending on operating 
system). If necessary, send these key files as an email attachment to 
the system administrator as part of a request for server/bridge access.

Moving to the bridge, as the administrator (or root), run 
:command:`sysadm-bridge import_ssl_file <nickname> <filepath>` for the 
requesting client's bridge key file. Now the client and bridge should be
able to communicate, but the client/server connection still needs to be 
established. 

On the server, run :command:`sudo sysadm-binary import_ssl_key <username> <filepath> [<email>]`
to import the client -> server SSL key file. This grants an individual 
with that specific SSL authorization the same permissions as <user>.

Back in the user client, open the connection manager and choose "Bridge 
Relay" as the connection option. Input the established bridge's URL and 
click "Connect".The bridge will now show up in the menu tree with a 
different icon, and will have a sub-menu of connections within it. If 
you click on the bridged system, it will open the standard UI but the 
connection is still being relayed through the bridge.

.. _adddoc:

Additional Documentation
------------------------

API documentation can be found at https://api.pcbsd.org .
