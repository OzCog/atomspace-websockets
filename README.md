# Obsolete
The code here is now obsolete. It has been replaced by "native" support
in the [CogServer](https://github.com/opencog/cogserver/). A JSON+websoockets
interface is provided on port 18080. See the websockets demo in the
[CogServer examples directory.](https://github.com/opencog/cogserver/tree/master/examples)
The code below was an older, earlier attempt to prove WebSockets.

# AtomSpace Websockets Server

This is a simple project that provides Websockets access to the OpenCog 
[AtomSpace](https://github.com/opencog/atomspace) to execute
 pattern matching queries. It also allows users to create multiple atomspaces and access them separately. Users can
  use a JSON config file to specify the id and the directory containing the atomese code to load. An example config
   looks like the following:
   
   ```json
    [
      {
        "id": "asp1",
        "pathDir": "/path/to/atomese/code/"
      },
      {
        "id": "asp2",
        "pathDir": "/path/to/another/atomese/code"
      }   
    ]
``` 

You can use the above JSON to start the server like this:

    $ atom_server --config /path/to/setting.json

This will create to AtomSpaces with ids`asp1` & `asp2` and can be accessed via WebSockets. An example Javascript to
 access an AtomSpace containing the `sample_dataset.scm` can be:
 
 ```html
    <!DOCTYPE html>
    <html>
        <body>
            <script type="text/javascript">
             var ws = new WebSocket("ws://localhost:9001/asp1"); //endpoint to access asp1 AtomSpace
             var patt = `(GetLink 
                            (EvaluationLink 
                                (Predicate "is_part")
                                (ListLink 
                                    (Concept "A")
                                    (Variable "$x"))))`; //this pattern searches for all ConceptNodes where A has
                                                         //is_part relationship
             console.log("initialized websocket");
             ws.onmessage = function(evt) {
               
                if(evt.data !== "eof"){
                    console.log("received message");
                    document.getElementById("res").innerHTML += "<p>" +  evt.data + "</p>";
                    console.log(evt);
                } else {
                    console.log("End of message received");
                }   
             };
             ws.onopen = function() {
                 console.log("connected");
                 //Send the pattern to be executed
                 ws.send(patt);
             };
             ws.onclose = function() {
                 console.log("closed websocket");
             }
            </script>
            <p id="res">Results are: </p>
        </body>
    </html>
```

### Building and Installation

#### 1. Requirements:

This project depends on the following libraries:

   - [AtomSpace](https://github.com/opencog/atomspace)
   - [Boost](https://www.boost.org/)
   - [uWebSockets](https://github.com/uNetworking/uWebSockets) -- make sure you check out the `v18` branch
       * Note: [uSockets](https://github.com/uNetworking/uSockets) library is needed which uWebSockets depends on it. After building the uSockets, you need to copy the static library to a library search path (e.g /usr/local/lib)
   - [nlohmann_json](https://github.com/nlohmann/json)
   - [Agi-bio](https://github.com/opencog/agi-bio) - this is OPTIONAL and will be required if you are working with
    bio related atomspace
    
#### 2. Building

```bash
$ cd atomspace_server
$ mkdir build && cd build
$ cmake ..
$ make 
$ make install
```
   
### Note

This project is inspired by the already existing [CogServer](https://github.com/opencog/cogserver). However
, this project is intended provide to a very simple, high-level access to the AtomSpace to run pattern matching queries
 via Websockets and the ability to load multiple atomspaces at once. It doesn't support the use Scheme or Python REPL to
  access an AtomSpace and you can't do anything complex(yet) such as job monitoring, loading other scheme modules
   ..etc that you can do with CogServer. If you are looking for such functionality please check out CogServer.
