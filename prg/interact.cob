
       data division.
       working-storage section.
       01 ws-name pic a(30) value LOW-VALUES.
       01 ws-header pic a(50) value "Please enter your name below:".

       01 ws-greet.
           05 filler pic aaaaa value "Hello".
           05 filler     pic aa    value ", ".
           05 ws-subject pic a(30) value SPACES.
           05 filler     pic a     value "!".
       
       procedure division.
       start.
           display ws-header.
           accept ws-name.
           move ws-name to ws-subject.
           display ws-greet.

