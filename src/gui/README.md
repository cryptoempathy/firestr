gui library
===================================================================

Implements the GUI and main functions of firestr. It connects most
of the pieces together and is probably the most HACKish part of the
project. 

file summary
===================================================================

mainwin       
-------------------------------------------------------------------

Creates the main window and starts everything up.

list          
-------------------------------------------------------------------
implements a simple list gui object.
             
message       
-------------------------------------------------------------------
implements the container for 'messages' to live in, 
which are added to list. 

conversation  
-------------------------------------------------------------------
implements a conversation gui, including code to add contacts
to a conversation and stores messages for that conversation.

messagelist   
-------------------------------------------------------------------
a factory class which creates a message based on type.
for example, if a app message comes it, it will initiate.
the application.

contactlist   
-------------------------------------------------------------------
UI for managing contacts and greeters.

contactselect 
-------------------------------------------------------------------
UI for select available contacts in a dropdown

mail_service  
-------------------------------------------------------------------

A thread which monitors a mailbox and emits a signal when a 
message arrives.

debugwin  
-------------------------------------------------------------------
Debug window which shows logs and connection graphs
