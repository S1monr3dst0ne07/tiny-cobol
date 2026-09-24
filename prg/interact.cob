
       data division.
       working-storage section.
       01 ws-greet.
           05 ws-header  pic aaaaa value "hello".
           05 filler     pic aa    value ", ".
           05 ws-subject pic a(10) value SPACES.
           05 filler     pic a     value "!".
       
       procedure division.
       start.
           move "joe mama" to ws-subject.
           display ws-greet.

