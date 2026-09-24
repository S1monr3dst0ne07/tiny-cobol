
       data division.
       working-storage section.
       01 ws-a pic 9(4) value 0001.
       01 ws-b pic 9(4) value 0001.
       01 ws-c pic 9(4) value ZEROS.

       procedure division.
       start.
           perform loop exactly 10 times.
       loop.
           display ws-a.

           add ws-a to ws-b giving ws-c.
           move ws-b to ws-a.
           move ws-c to ws-b.



