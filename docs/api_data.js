define({ "api": [
  {
    "type": "get",
    "url": "/get_log_list",
    "title": "get_log_list",
    "version": "1.0.0",
    "name": "get_log_list",
    "group": "Administration",
    "permission": [
      {
        "name": "user"
      }
    ],
    "description": "<p>Returns shorted registered logs, where the shorting criteria depends on the  provided &quot;sorttype&quot; parameter.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "int",
            "optional": false,
            "field": "sorttype",
            "description": "<p>Shorting criteria (OPTIONAL PARAMETER): 1: sort by id 2: sort by error code 3: sort by user 4: sort by IP address 5: sort by Message 0: sort by Date: default if not provided code</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "pretty",
            "description": "<p>For tabulate the json response and make it more human readable.</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -XGET  http://serveraddress:server:port/get_log_list?sorttype=\"XXX\"\\&pretty=\"true\"\nor alternatively use service at https when available SSL certificates.",
        "type": "json"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "response",
            "description": "<p>JSON structure</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success response:",
          "content": "HTTPS 200 OK\n{....}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/app_sec.js",
    "groupTitle": "Administration"
  },
  {
    "type": "post",
    "url": "/new_log",
    "title": "Registers a log.",
    "version": "1.0.0",
    "name": "new_log",
    "group": "Administration",
    "permission": [
      {
        "name": "user"
      }
    ],
    "description": "<p>Registers a log with the provided input parameters.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "code",
            "description": "<p>sucecss/error HTML code of the event.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "user_id",
            "description": "<p>Id of the user responsible of the event.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "ip",
            "description": "<p>The ip from where the user ran the event. If not provided will store the ip from where is requested the new log.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "message",
            "description": "<p>The description of the event.</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -H \"Content-Type: text/plain\" -XPOST http://server:port/new_log?code=111\\&ip=\"10.11.12.13\"\\&message=\"Some description\"\\&user=\"jaja@abc.com\"\nor alternatively use service at https when available SSL certificates.",
        "type": "json"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "response",
            "description": "<p>JSON structure</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success response:",
          "content": "HTTPS 200 OK\nregistered log",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InvalidToken",
            "description": "<p>Error message when the token is not valid</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Response (example):",
          "content": "HTTP/1.1 401 Not Authenticated\n  Invalid token",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/app_sec.js",
    "groupTitle": "Administration"
  },
  {
    "type": "post",
    "url": "/signup",
    "title": "Register a new user and its password.",
    "version": "1.0.0",
    "name": "signup",
    "group": "Administration",
    "permission": [
      {
        "name": "user"
      }
    ],
    "description": "<p>Update the password of an user</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "email",
            "description": "<p>MANDATORY this is the user_id.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "pw",
            "description": "<p>MANDATORY it is the password of the user</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -H \"Content-Type: text/plain\" -XPOST http://server:port/signup?name=\"bob\"\\&email=\"bob@abc.commm\"\\&pw=\"1234\";\n \nor alternatively use service at https when available SSL certificates.",
        "type": "json"
      }
    ],
    "success": {
      "examples": [
        {
          "title": "Success response:",
          "content": "HTTPS 200 OK",
          "type": "json"
        }
      ]
    },
    "error": {
      "examples": [
        {
          "title": "Response when missing mandatory parameters:",
          "content": "HTTP/1.1 400 Bad Request",
          "type": "json"
        },
        {
          "title": "Response when not valid user/password:",
          "content": "HTTP/1.1 401 Not Authenticated",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/app_sec.js",
    "groupTitle": "Administration"
  },
  {
    "type": "post",
    "url": "/update_user",
    "title": "Update the password of an user",
    "version": "1.0.0",
    "name": "update_user",
    "group": "Administration",
    "permission": [
      {
        "name": "user"
      }
    ],
    "description": "<p>Update the password of an user</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "email",
            "description": "<p>MANDATORY this is the user_id.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "pw",
            "description": "<p>MANDATORY it is the password of the user</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -H \"Content-Type: text/plain\" -XPOST http://server:port/update_user?name=\"bob\"\\&email=\"bob@abc.commm\"\\&pw=\"1234\";\n \nor alternatively use service at https when available SSL certificates.",
        "type": "json"
      }
    ],
    "success": {
      "examples": [
        {
          "title": "Success response:",
          "content": "HTTPS 200 OK",
          "type": "json"
        }
      ]
    },
    "error": {
      "examples": [
        {
          "title": "Response when missing mandatory parameters:",
          "content": "HTTP/1.1 400 Bad Request",
          "type": "json"
        },
        {
          "title": "Response when not valid user/password:",
          "content": "HTTP/1.1 401 Not Authenticated",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/app_sec.js",
    "groupTitle": "Administration"
  },
  {
    "type": "get",
    "url": "/experiments",
    "title": "1. Get a list of all available experiments",
    "version": "1.0.0",
    "name": "GetExperiments",
    "group": "Experiments",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "workflow",
            "description": "<p>filters results by the given workflow, e.g. 'ms2'</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/experiments",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "executionID",
            "description": "<p>Identifier of an experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "executionID.href",
            "description": "<p>Link to the experiment's details</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"AVZ-ll9FGYwmTvCuSnjW\": {\n     \"href\": \"http://localhost:3033/v1/phantom_mf/experiments/AVZ-ll9FGYwmTvCuSnjW?workflow=ms2\"\n  },\n  \"AVZ-kZTjGYwmTvCuSnZV\": {\n     \"href\": \"http://localhost:3033/v1/phantom_mf/experiments/AVZ-kZTjGYwmTvCuSnZV?workflow=ms2\"\n  },\n  \"AVZ-j2hEGYwmTvCuSnVE\": {\n     \"href\": \"http://localhost:3033/v1/phantom_mf/experiments/AVZ-j2hEGYwmTvCuSnVE?workflow=ms2\"\n  },\n  ...\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "ExperimentsNotAvailable",
            "description": "<p>No experiments found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 404 Not Found\n{\n  \"error\": \"No experiments found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/experiments.js",
    "groupTitle": "Experiments"
  },
  {
    "type": "get",
    "url": "/experiments/:experimentID",
    "title": "2. Get a registered experiment with given execution ID",
    "version": "1.0.0",
    "name": "GetExperimentsByID",
    "group": "Experiments",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "experimentID",
            "description": "<p>Identifier of an experiment</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/experiments/AVZ-ll9FGYwmTvCuSnjW?workflow=ms2",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": true,
            "field": "application",
            "description": "<p>Name of the workflow</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": true,
            "field": "task",
            "description": "<p>Name of the task (sub-component of the workflow)</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": true,
            "field": "host",
            "description": "<p>Name of the target platform, where the experiment is conducted</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": true,
            "field": "timestamp",
            "description": "<p>Timestamp when the experiment is registered</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n   \"application\": \"ms2\",\n   \"task\": \"t1\",\n   \"host\": \"node01\",\n   \"@timestamp\": \"2016-08-12T13:49:59\"\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "DatabaseError",
            "description": "<p>Elasticsearch specific error message.</p>"
          }
        ]
      }
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/experiments.js",
    "groupTitle": "Experiments"
  },
  {
    "type": "post",
    "url": "/experiments/:workflowID",
    "title": "3. Create a new experiment with given workflow ID",
    "version": "1.0.0",
    "name": "PostExperiments",
    "group": "Experiments",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier for the workflow for which the experiment shall be created, e.g. 'ms2'</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "application",
            "description": "<p>Name of the application, same as the workflow ID</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "host",
            "description": "<p>Hostname of the target platform</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "author",
            "description": "<p>Author, like who is registering the experiment</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Request-Example:",
          "content": "{\n  \"application\": \"vector_scal01\",\n  \"host\": \"node01\",\n  \"author\": \"jmmontanana\"\n}",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/experiments/ms2",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "executionID",
            "description": "<p>Identifier of the new registered experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "executionID.href",
            "description": "<p>Link to the new registered experiment</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"AVXt3coOz5chEwIt8_Ma\": {\n    \"href\": \"http://localhost:3033/v1/phantom_mf/experiments/AVXt3coOz5chEwIt8_Ma?workflow=ms2\"\n  }\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "WorkflowNotFound",
            "description": "<p>No workflow as given is found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 404 Not Found\n{\n  \"error\": \"No workflow as '\" + workflowID + \"' is found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/experiments.js",
    "groupTitle": "Experiments"
  },
  {
    "type": "get",
    "url": "/metrics/:workflowID/:taskID/:experimentID",
    "title": "1. Get all sampled metrics' names and average values with given workflowID, taskID and experimentID",
    "version": "1.0.0",
    "name": "GetMetrics",
    "group": "Metrics",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Name of the workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "taskID",
            "description": "<p>Name of the task</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "executionID",
            "description": "<p>Identifier of the experiment</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/metrics/ms2/t1/AVNXMXcvGMPeuCn4bMe0",
        "type": "curl"
      }
    ],
    "success": {
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n[\n  {\n    \"device0:current\":\n       {\n         \"count\":55,\n         \"min\":0,\n         \"max\":0,\n         \"avg\":0,\n         \"sum\":0\n       },\n    \"device0:vbus\":\n       {\n         \"count\":55,\n         \"min\":0,\n         \"max\":0,\n         \"avg\":0,\n         \"sum\":0\n       },\n     \"device0:vshunt\":\n       {\n         \"count\":55,\n         \"min\":0,\n         \"max\":0,\n         \"avg\":0,\n         \"sum\":0\n       },\n     \"device0:power\":\n       {\n         \"count\":55,\n         \"min\":0,\n         \"max\":0,\n         \"avg\":0,\n         \"sum\":0\n       }\n  }\n]",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "DatabaseError",
            "description": "<p>Elasticsearch specific error message.</p>"
          }
        ]
      }
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/metrics.js",
    "groupTitle": "Metrics"
  },
  {
    "type": "post",
    "url": "/metrics",
    "title": "3. Send an array of metrics",
    "version": "1.0.0",
    "name": "PostBulkMetrics",
    "group": "Metrics",
    "parameter": {
      "fields": {
        "body": [
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "WorkflowID",
            "description": "<p>Name of the application</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "TaskID",
            "description": "<p>Name of the task</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "ExperimentID",
            "description": "<p>Identifier of the experiment</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": true,
            "field": "type",
            "description": "<p>Type of the metric, e.g. power, temperature, and so on</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": true,
            "field": "host",
            "description": "<p>Hostname of the target platform</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "local_timestamp",
            "description": "<p>Local timestamp, when the metric is collected</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>Name and value of the metric</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Request-Example:",
          "content": "[\n  {\n    \"WorkflowID\": \"ms2\",\n    \"ExperimentID\": \"AVUWnydqGMPeuCn4l-cj\",\n    \"TaskID\": \"t2.1\",\n    \"local_timestamp\": \"2016-02-15T12:43:48.749\",\n    \"type\": \"power\",\n    \"host\": \"node01.excess-project.eu\",\n    \"GPU1:power\": \"168.519\"\n  }, {\n    \"WorkflowID\": \"ms2\",\n    \"ExperimentID\":\"AVNXMXcvGMPeuCn4bMe0\",\n    \"TaskID\": \"t2.2\",\n    \"local_timestamp\": \"2016-02-15T12:46:48.524\",\n    \"type\": \"power\",\n    \"host\": \"node01.excess-project.eu\",\n    \"GPU0:power\": \"152.427\"\n  }\n]",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/metrics",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "href",
            "description": "<p>links to all updated profiled metrics</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n[\n  \"http://localhost:3033/v1/phantom_mf/profiles/ms2/t2.1/AVUWnydqGMPeuCn4l-cj\",\n  \"http://localhost:3033/v1/phantom_mf/profiles/ms2/t2.2/AVNXMXcvGMPeuCn4bMe0\"\n]",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "DatabaseError",
            "description": "<p>Elasticsearch specific error message.</p>"
          }
        ]
      }
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/metrics.js",
    "groupTitle": "Metrics"
  },
  {
    "type": "post",
    "url": "/metrics/:workflowID/:taskID/:experimentID",
    "title": "2. Send a metric with given workflow ID, task ID, and experiment ID",
    "version": "1.0.0",
    "name": "PostMetric",
    "group": "Metrics",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "WorkflowID",
            "description": "<p>Name of the application</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "TaskID",
            "description": "<p>Name of the task</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "ExperimentID",
            "description": "<p>Identifier of the experiment</p>"
          }
        ],
        "body": [
          {
            "group": "body",
            "type": "String",
            "optional": true,
            "field": "type",
            "description": "<p>Type of the metric, e.g. power, temperature, and so on</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": true,
            "field": "host",
            "description": "<p>Hostname of the target platform</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "local_timestamp",
            "description": "<p>Local timestamp, when the metric is collected</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>Name and value of the metric</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Request-Example:",
          "content": "{\n  \"type\": \"power\",\n  \"host\": \"node01.excess-project.eu\",\n  \"local_timestamp\": \"2016-02-15T12:42:22.000\",\n  \"GPU0:power\": \"152.427\"\n}",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/metrics/ms2/t1/AVNXMXcvGMPeuCn4bMe0",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "metricID",
            "description": "<p>Identifier of the sent metric</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "metricID.href",
            "description": "<p>Link to the experiment with updated metrics</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"AVXt3coOz5chEwIt8_Ma\": {\n    \"href\": \"http://localhost:3033/v1/phantom_mf/profiles/ms2/t1/AVNXMXcvGMPeuCn4bMe0\"\n  }\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "DatabaseError",
            "description": "<p>Elasticsearch specific error message.</p>"
          }
        ]
      }
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/metrics.js",
    "groupTitle": "Metrics"
  },
  {
    "type": "get",
    "url": "/profiles/:workflowID/:taskID/:experimentID",
    "title": "3. Get the profiled metrics with given workflow ID, task ID and experiment ID",
    "version": "1.0.0",
    "name": "GetProfilesExperiment",
    "group": "Profiles",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "taskID",
            "description": "<p>Identifier of a registered task</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "experimentID",
            "description": "<p>Identifier of an experiment</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "from",
            "description": "<p>Time filter, starting point of the metrics collection time (if not given, starting point is 5 min from current time)</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "to",
            "description": "<p>Time filter, ending point of the metrics collection time (if not given, ending point is the current time)</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/profiles/dummy/t1/AVSbT0ChGMPeuCn4QYjq",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "Metrics",
            "description": "<p>Measurements of an experiment based on the system</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "Metrics.local_timestamp",
            "description": "<p>Local time, when the metric data is collected</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "Metrics.server_timestamp",
            "description": "<p>Server time, when the metric data is received by the server</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "Metrics.host",
            "description": "<p>Hostname of the target system</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "Metrics.type",
            "description": "<p>Type of the metric, e.g. power, temperature, and so on</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "Metrics.metric",
            "description": "<p>Name and value of the metric</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n[\n    {\n     \"local_timestamp\":\"2016-05-10T17:35:59.576\",\n     \"server_timestamp\":\"2016-05-10T17:36:01.541\",\n     \"host\":\"node01.excess-project.eu\",\n     \"type\":\"energy\",\n     \"DRAM_ENERGY:PACKAGE0\":1.5715,\n     \"DRAM_POWER:PACKAGE0\":1.571,\n    },{\n     \"local_timestamp\":\"2016-05-10T17:35:59.708\",\n     \"server_timestamp\":\"2016-05-10T17:36:01.541\",\n     \"host\":\"node01.excess-project.eu\",\n     \"type\":\"memory\",\n     \"MemTotal\":32771284,\n     \"MemFree\":31720604\n    },{\n     \"local_timestamp\":\"2016-05-10T17:35:59.831\",\n     \"server_timestamp\":\"2016-05-10T17:36:01.541\",\n     \"host\":\"node01.excess-project.eu\",\n     \"type\":\"temperature\",\n     \"CPU1_Core 1\":30,\n     \"CPU1_Core 0\":25\n    }\n]",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InternalSeverError",
            "description": "<p>No results found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Sever Error\n{\n  \"error\": \"No results found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/profiles.js",
    "groupTitle": "Profiles"
  },
  {
    "type": "get",
    "url": "/profiles/:workflowID/:taskID",
    "title": "2. Get a list of the profiled experiments with given workflow ID and task ID",
    "version": "1.0.0",
    "name": "GetProfilesTask",
    "group": "Profiles",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "taskID",
            "description": "<p>Identifier of a registered task</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/profiles/dummy/t1",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "date",
            "description": "<p>Date, when the task is registered</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "date.experimentID",
            "description": "<p>Identifier of an experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "date.experimentID.href",
            "description": "<p>Link to the experiment</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"2016-05-11\":{\n     \"AVSf5_wVGMPeuCn4Qdw2\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t1/AVSf5_wVGMPeuCn4Qdw2\"\n     },\n     \"AVSf-mU4GMPeuCn4Qd0L\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t1/AVSf-mU4GMPeuCn4Qd0L\"\n     }\n  },\n  \"2016-05-10\":{\n     \"AVXAMB5FLeaeU4rxyi3w\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t1/AVXAMB5FLeaeU4rxyi3w\"\n     },\n     \"AVVT4dhwenoRsEhyDkeb\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t1/AVVT4dhwenoRsEhyDkeb\"\n     }\n  }\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InternalSeverError",
            "description": "<p>No results found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Sever Error\n{\n  \"error\": \"Task not found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/profiles.js",
    "groupTitle": "Profiles"
  },
  {
    "type": "get",
    "url": "/profiles/:workflowID",
    "title": "1. Get a list of the profiled tasks and experiments with given workflow ID",
    "version": "1.0.0",
    "name": "GetProfilesWorkflow",
    "group": "Profiles",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/profiles/dummy",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "taskID",
            "description": "<p>Identifier of a registered task</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "taskID.experimentID",
            "description": "<p>Identifier of an experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "taskID.experimentID.href",
            "description": "<p>Link to the experiment</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"t1\":{\n     \"AVSf5_wVGMPeuCn4Qdw2\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t1/AVSf5_wVGMPeuCn4Qdw2\"\n     },\n     \"AVSf-mU4GMPeuCn4Qd0L\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t1/AVSf-mU4GMPeuCn4Qd0L\"\n     }\n  },\n  \"t2\":{\n     \"AVXAMB5FLeaeU4rxyi3w\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t2/AVXAMB5FLeaeU4rxyi3w\"\n     },\n     \"AVVT4dhwenoRsEhyDkeb\":{\n           \"href\":\"http://localhost:3033/v1/phantom_mf/profiles/dummy/t2/AVVT4dhwenoRsEhyDkeb\"\n     }\n  }\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InternalSeverError",
            "description": "<p>No results found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Sever Error\n{\n  \"error\": \"No results found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/profiles.js",
    "groupTitle": "Profiles"
  },
  {
    "type": "get",
    "url": "/configs",
    "title": "1. Get a list of configurations of all platforms",
    "version": "1.0.0",
    "name": "GetConfigs",
    "group": "RM_Configs",
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "platformID",
            "description": "<p>Unique platform identifier</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "platformID.status",
            "description": "<p>Status of the plugin (on/off)</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "platformID.sampling_interval",
            "description": "<p>Sampling interval of the plugin (in nanosecond)</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "platformID.metrics",
            "description": "<p>Name and status (on/off) of metrics of the plugin</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n     \"node01\":{\n         \"mf_plugin_Board_power\":{\n             \"status\":\"on\",\n             \"sampling_interval\":\"1000000000ns\",\n             \"device0:current\":\"on\",\n             \"device0:vshunt\":\"on\",\n             \"device0:vbus\":\"on\",\n             \"device0:power\":\"on\"},\n         \"mf_plugin_CPU_perf\":{\n             \"status\":\"on\",\n             \"sampling_interval\":\"1000000000ns\",\n             \"MIPS\":\"on\"},\n         \"mf_plugin_CPU_temperature\":{\n             \"status\":\"on\",\n             \"sampling_interval\":\"1000000000ns\",\n             \"CPU0:core0\":\"off\"}\n     },\n     \"movidius\":{\n         \"power\":{\n             \"status\":\"on\",\n             \"sampling_interval\":\"1000000000ns\"},\n         \"temperature\":{\n             \"status\":\"on\",\n             \"sampling_interval\":\"1000000000ns\"}\n     }\n }",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_rm/configs",
        "type": "curl"
      }
    ],
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "ConfigsNotAvailable",
            "description": "<p>No configurations found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 404 Not Found\n{\n  \"error\": \"No configurations found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/configs.js",
    "groupTitle": "RM_Configs"
  },
  {
    "type": "get",
    "url": "/configs/:platformID",
    "title": "2. Get the configuration of a specific platform",
    "version": "1.0.0",
    "name": "GetConfigsByPlatformID",
    "group": "RM_Configs",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "platformID",
            "description": "<p>Unique platform identifier</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_rm/configs/node01.excess-cluster",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "body": [
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "status",
            "description": "<p>Status of the plugin (on/off)</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "sampling_interval",
            "description": "<p>Sampling interval of the plugin (in nanosecond)</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "metrics",
            "description": "<p>Name and status (on/off) of metrics of the plugin</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n     \"mf_plugin_Board_power\": {\n         \"status\": \"on\",\n         \"sampling_interval\": \"1000000000ns\",\n         \"device0:current\": \"on\",\n         \"device0:vshunt\": \"on\",\n         \"device0:vbus\": \"off\",\n         \"device0:power\": \"on\" }, \n     \"mf_plugin_CPU_perf\": {\n         \"status\": \"on\",\n         \"sampling_interval\": \"1000000000ns\",\n         \"MIPS\": \"on\" }, \n     \"mf_plugin_CPU_temperature\": {\n         \"status\": \"on\",\n         \"sampling_interval\": \"1000000000ns\",\n         \"CPU0:core0\": \"on\"}\n }",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "PlatformNotAvailable",
            "description": "<p>Given platformID does not exist.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 404 Not Found\n{\n  \"error\": \"Configuration for the platform'\" + platformID + \"' is not found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/configs.js",
    "groupTitle": "RM_Configs"
  },
  {
    "type": "put",
    "url": "/configs/:platformID",
    "title": "3. Add/Updata the configuration of a specific platform",
    "version": "1.0.0",
    "name": "PutConfigs",
    "group": "RM_Configs",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "platformID",
            "description": "<p>Unique platform identifier</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "status",
            "description": "<p>Status of the plugin (on/off)</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "sampling_interval",
            "description": "<p>Sampling interval of the plugin (in nanosecond)</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "metrics",
            "description": "<p>Name and status (on/off) of metrics of the plugin</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Request-Example:",
          "content": "{\n     \"mf_plugin_Board_power\": {\n         \"status\": \"on\",\n         \"sampling_interval\": \"1000000000ns\",\n         \"device0:current\": \"on\",\n         \"device0:vshunt\": \"on\",\n         \"device0:vbus\": \"off\",\n         \"device0:power\": \"on\" }, \n     \"mf_plugin_CPU_perf\": {\n         \"status\": \"on\",\n         \"sampling_interval\": \"1000000000ns\",\n         \"MIPS\": \"on\" }, \n     \"mf_plugin_CPU_temperature\": {\n         \"status\": \"on\",\n         \"sampling_interval\": \"1000000000ns\",\n         \"CPU0:core0\": \"on\"}\n }",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_rm/configs/node01.excess-cluster",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "href",
            "description": "<p>Link to the stored configuration resource</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"href\": \"http://localhost:3033/v1/phantom_rm/configs/node01.excess-cluster\",\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "StorageError",
            "description": "<p>Given configuration could not be stored.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Server Error\n{\n  \"error\": \"Could not change the configuration.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/configs.js",
    "groupTitle": "RM_Configs"
  },
  {
    "type": "get",
    "url": "/resources",
    "title": "1. Get a list of resources links and avaiable platforms",
    "version": "1.0.0",
    "name": "GetResources",
    "group": "RM_Resources",
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_rm/resources",
        "type": "curl"
      }
    ],
    "success": {
      "examples": [
        {
          "title": "Success-Response:",
          "content": " HTTP/1.1 200 OK\n {\n\t\"alexlaptop\": {\n\t   \"href\": \"http://localhost:3033/v1/phantom_rm/resources/alexlaptop\"\n\t},\n\t\"movidius\": {\n\t   \"href\": \"http://localhost:3033/v1/phantom_rm/resources/movidius\"\n\t},\n\t\"excesscluster\": {\n\t   \"href\": \"http://localhost:3033/v1/phantom_rm/resources/excesscluster\"\n\t}\n }",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InternalSeverError",
            "description": "<p>No results found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Sever Error\n{\n  \"error\": \"No resources found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/resources.js",
    "groupTitle": "RM_Resources"
  },
  {
    "type": "get",
    "url": "/resources/:platformID",
    "title": "2. Get resource information for a given platform",
    "version": "1.0.0",
    "name": "GetResourcesByPlatformID",
    "group": "RM_Resources",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "platformID",
            "description": "<p>Unique platform identifier</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_rm/resources/excesscluster",
        "type": "curl"
      }
    ],
    "success": {
      "examples": [
        {
          "title": "Success-Response:",
          "content": " HTTP/1.1 200 OK\n {\n\t\"nodes\": [\n\t   {\n\t\t  \"id\": \"node01\",\n\t\t  \"cpus\": [\n\t\t\t {\n\t\t\t\t\"id\": \"cpu0\",\n\t\t\t\t\"cores\": [\n\t\t\t\t   {\n\t\t\t\t\t  \"id\": \"core0\",\n\t\t\t\t\t  \"pwMode\": 0,\n\t\t\t\t\t  \"status\": \"allocated\",\n\t\t\t\t\t  \"availTime\": \"2016-10-21T14:56:02.304\"\n\t\t\t\t   }, {\n\t\t\t\t\t  \"id\": \"core1\",\n\t\t\t\t\t  \"pwMode\": 0,\n\t\t\t\t\t  \"status\": \"allocated\",\n\t\t\t\t\t  \"availTime\": \"2016-10-21T15:21:07.567\"\n\t\t\t\t   },\n\t\t\t\t   ...\n\t\t\t\t]\n\t\t\t }\n\t\t  ]\n\t   }\n\t]\n }",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "PlatformNotAvailable",
            "description": "<p>Given platformID does not exist.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 404 Not Found\n{\n  \"error\": \"Configuration for the platform is not found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/resources.js",
    "groupTitle": "RM_Resources"
  },
  {
    "type": "put",
    "url": "/resources/:platformID",
    "title": "3. Add/Update resource information for a given platform",
    "version": "1.0.0",
    "name": "PutResources",
    "group": "RM_Resources",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "platformID",
            "description": "<p>Unique platform identifier</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "id",
            "description": "<p>Unique identifier of the resource</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "pwMode",
            "description": "<p>Power mode of the resource</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "status",
            "description": "<p>Status of the resource (allocated/free)</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "availTime",
            "description": "<p>Until when the resource will be free</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Request-Example:",
          "content": " {\n\t\"nodes\": [\n\t   {\n\t\t  \"id\": \"node01\",\n\t\t  \"cpus\": [\n\t\t\t {\n\t\t\t\t\"id\": \"cpu0\",\n\t\t\t\t\"cores\": [\n\t\t\t\t\t{\n\t\t\t\t\t  \"id\": \"core0\",\n\t\t\t\t\t  \"pwMode\": 0,\n\t\t\t\t\t  \"status\": \"allocated\",\n\t\t\t\t\t  \"availTime\": \"2016-10-21T14:56:02.304\"\n\t\t\t\t\t}, {\n\t\t\t\t\t  \"id\": \"core1\",\n\t\t\t\t\t  \"pwMode\": 0,\n\t\t\t\t\t  \"status\": \"allocated\",\n\t\t\t\t\t  \"availTime\": \"2016-10-21T15:21:07.567\"\n\t\t\t\t\t},\n\t\t\t\t\t...\n\t\t\t\t]\n\t\t\t }\n\t\t  ]\n\t   }\n\t]\n }",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_rm/resources/excesscluster",
        "type": "curl"
      }
    ],
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "StorageError",
            "description": "<p>Given resources could not be stored.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Server Error\n{\n  \"error\": \"Could not change the resources.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/resources.js",
    "groupTitle": "RM_Resources"
  },
  {
    "type": "get",
    "url": "/runtime/:workflowID/:taskID/:experimentID",
    "title": "2. Get runtime information with given workflow ID, task ID and experiment ID",
    "version": "1.0.0",
    "name": "GetRuntime",
    "group": "Runtime",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "taskID",
            "description": "<p>Identifier of a task</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "experimentID",
            "description": "<p>Identifier of an experiment</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/runtime/ms2/t1/AVSbT0ChGMPeuCn4QYjq",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "start",
            "description": "<p>Start timestamp of the specific task and experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "end",
            "description": "<p>End timestamp of the specific task and experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "runtime",
            "description": "<p>Duration of the experiment in seconds</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "host",
            "description": "<p>Hostname of the system</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n     \"start\":\"2016-05-10T17:35:49.125\",\n     \"end\":\"2016-05-10T17:36:01.749\",\n     \"runtime\":12.624000072479248,\n     \"host\":\"node01.excess-project.eu\"\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InternalSeverError",
            "description": "<p>No results found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Sever Error\n{\n  \"error\": \"No results found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/runtime.js",
    "groupTitle": "Runtime"
  },
  {
    "type": "get",
    "url": "/runtime/:workflowID/:experimentID",
    "title": "1. Get runtime information with given workflow ID and experiment ID",
    "version": "1.0.0",
    "name": "GetRuntimeByExperiment",
    "group": "Runtime",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "experimentID",
            "description": "<p>Identifier of an experiment</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/runtime/ms2/AVZ-5cqVGYwmTvCuSqZC",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "workflow",
            "description": "<p>Identifier of the workflow</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "start",
            "description": "<p>Start local timestamp of the entire experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "end",
            "description": "<p>End local timestamp of the entire experiment</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "total_runtime",
            "description": "<p>Duration of the entire experiment in seconds</p>"
          },
          {
            "group": "Success 200",
            "type": "Array",
            "optional": false,
            "field": "tasks",
            "description": "<p>Array of task-specific runtime information</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "tasks.task",
            "description": "<p>Identifier of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "tasks.data",
            "description": "<p>Object holding runtime data of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "tasks.data.start",
            "description": "<p>Start local timestamp of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "tasks.data.end",
            "description": "<p>End local timestamp of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "tasks.data.runtime",
            "description": "<p>Duration of the task in seconds</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n   \"workflow\": \"ms2\",\n   \"tasks\": [\n      ...\n      {\n         \"task\": \"T2.1\",\n         \"data\": {\n            \"start\": \"2016-08-12T15:20:40.631\",\n            \"end\": \"2016-08-12T15:21:22.205\",\n            \"runtime\": 41.574\n         }\n      },\n      {\n         \"task\": \"T2.2\",\n         \"data\": {\n            \"start\": \"2016-08-12T15:21:46.975\",\n            \"end\": \"2016-08-12T15:22:25.983\",\n            \"runtime\": 39.008\n         }\n      },\n      ...\n   ],\n   \"start\": \"2016-08-12T15:17:46.731\",\n   \"end\": \"2016-08-12T15:25:30.452\",\n   \"total_runtime\": 463.721\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InternalServerError",
            "description": "<p>No results found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Server Error\n{\n  \"error\": \"No results found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/runtime.js",
    "groupTitle": "Runtime"
  },
  {
    "type": "post",
    "url": "/login",
    "title": "Returns a new token",
    "version": "1.0.0",
    "name": "login",
    "group": "Security",
    "permission": [
      {
        "name": "user"
      }
    ],
    "description": "<p>Returns a new encrypted token, with a limited lifetime, for the user_id if the user_id/password provided are valid.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "email",
            "description": "<p>MANDATORY this is the user_id.</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "pw",
            "description": "<p>MANDATORY it is the password of the user</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -H \"Content-Type: text/plain\" -XGET http://localhost:8000/login?email=\"bob\"\\&pw=\"1234\" --output token.txt;\n \nor alternatively use service at https when available SSL certificates.",
        "type": "json"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "response",
            "description": "<p>token which consists on a encrypted text string.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success response:",
          "content": "HTTPS 200 OK",
          "type": "json"
        }
      ]
    },
    "error": {
      "examples": [
        {
          "title": "Response when missing mandatory parameters:",
          "content": "HTTP/1.1 400 Bad Request",
          "type": "json"
        },
        {
          "title": "Response when not valid user/password:",
          "content": "HTTP/1.1 401 Not Authenticated",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/app_sec.js",
    "groupTitle": "Security"
  },
  {
    "type": "get",
    "url": "/verifytoken",
    "title": "Returns if the provided Token is valid or not",
    "version": "1.0.0",
    "name": "verifytoken",
    "group": "Security",
    "permission": [
      {
        "name": "user"
      }
    ],
    "description": "<p>Verifies of the provided token is valid. Authorization Tokens are generated when user autenticates with an id and a password, the tokens are required for accessing to private content only if autenticated.</p>",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "the",
            "description": "<p>token.</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -s -H \"Authorization: OAuth ${mytoken}\" -XGET http://${server}:${port}/verifytoken\nor alternatively when available SSL certificates:\ncurl -s -H \"Authorization: OAuth ${mytoken}\" -XGET https://${server}:${port}/verifytoken",
        "type": "json"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "reponse",
            "description": "<p>The token is valid !!!.</p>"
          }
        ]
      }
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "InvalidToken",
            "description": "<p>Invalid token</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Response (example):",
          "content": "HTTP/1.1 401 Not Authenticated\n  Invalid token",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/app_sec.js",
    "groupTitle": "Security"
  },
  {
    "type": "get",
    "url": "/statistics/:workflowID",
    "title": "1. Get statistics of a metric across all tasks and experiments with given workflow ID",
    "version": "1.0.0",
    "name": "GetStats",
    "group": "Statistics",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>Name of a metric, e.g., metric=CPU0:core1</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "host",
            "description": "<p>Hostname of the system, e.g., host=node01</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "from",
            "description": "<p>Start time of the statistics, e.g., from=2016-05-10T17:35:57.610</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "to",
            "description": "<p>End time of the statistics, e.g., to=2016-05-10T17:35:57.610</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i 'http://localhost:3033/v1/phantom_mf/statistics/ms2?metric=CPU0:core1'",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "workflow",
            "description": "<p>workflow-related data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "workflow.href",
            "description": "<p>link to the stored workflow information</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>name of the metric for which statistics are captured</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "statistics",
            "description": "<p>extended set of statistics as provided by Elasticsearch</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.count",
            "description": "<p>number of metric values sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.min",
            "description": "<p>minimum value obtained for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.max",
            "description": "<p>maximum value obtained for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.avg",
            "description": "<p>average value across all metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.sum",
            "description": "<p>sum of all sampled metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.sum_of_squares",
            "description": "<p>sum of squares for the given metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.variance",
            "description": "<p>variance of the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation",
            "description": "<p>standard deviation computed for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "statistics.std_deviation_bounds",
            "description": "<p>deviation bounds of the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation_bounds.upper",
            "description": "<p>deviation upper bounds</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation_bounds.lower",
            "description": "<p>deviation lower bounds</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "min",
            "description": "<p>minimum point of the metrics data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.local_timestamp",
            "description": "<p>local time, when the the minimum metric value is sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.server_timestamp",
            "description": "<p>server time, when the the minimum metric value is received by the server</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.host",
            "description": "<p>hostname of the target platform</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.TaskID",
            "description": "<p>identifier of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.type",
            "description": "<p>type of plug-in the metric is associated with</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.metric",
            "description": "<p>metric value associated with a given metric name</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "max",
            "description": "<p>maximum point of the metrics data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.local_timestamp",
            "description": "<p>local time, when the the maximum metric value is sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.host",
            "description": "<p>hostname of the target platform</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.TaskID",
            "description": "<p>identifier of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.type",
            "description": "<p>type of plug-in the metric is associated with</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.metric",
            "description": "<p>metric value associated with a given metric name</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n   \"workflow\": {\n      \"href\": \"http://localhost:3033/v1/phantom_mf/workflows/ms2\"\n   },\n   \"metric\": \"CPU0:core1\",\n   \"statistics\": {\n      \"count\": 1272,\n      \"min\": 25,\n      \"max\": 30,\n      \"avg\": 27.19575471698,\n      \"sum\": 34593,\n      \"sum_of_squares\": 941499,\n      \"variance\": 0.56309518,\n      \"std_deviation\": 0.750396685173416,\n      \"std_deviation_bounds\": {\n         \"upper\": 28.69654808732796,\n         \"lower\": 25.6949613466343\n      }\n   },\n   \"min\": {\n      \"@timestamp\": \"2016-05-17T16:25:48.123\",\n      \"host\": \"node01.excess-project.eu\",\n      \"task\": \"t1\",\n      \"type\": \"performance\",\n      \"CPU0:core0\": 26,\n      \"CPU0:core1\": 25,\n      ...\n   },\n   \"max\": {\n      ...\n   }\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "NoResults",
            "description": "<p>response is empty for the metric.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"error\": \"response is empty for the metric.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/statistics.js",
    "groupTitle": "Statistics"
  },
  {
    "type": "get",
    "url": "/statistics/:workflowID/:taskID/:experimentID",
    "title": "3. Get statistics of a metric with given workflow ID, task ID and experiment ID",
    "version": "1.0.0",
    "name": "GetStatsExperiment",
    "group": "Statistics",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "taskID",
            "description": "<p>Identifier of a task</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "experimentID",
            "description": "<p>Identifier of an experiment</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>Name of a metric, e.g., metric=CPU0:core1</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "host",
            "description": "<p>Hostname of the system, e.g., host=node01</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "from",
            "description": "<p>Start time of the statistics, e.g., from=2016-05-10T17:35:57.610</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "to",
            "description": "<p>End time of the statistics, e.g., to=2016-05-10T17:35:57.610</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i 'http://localhost:3033/v1/phantom_mf/statistics/ms2/t1/AVqkW4L57rO13ZBQKOWJ?metric=metric=CPU0:core1'",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "workflow",
            "description": "<p>workflow-related data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "workflow.href",
            "description": "<p>link to the stored workflow information</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>name of the metric for which statistics are captured</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "statistics",
            "description": "<p>extended set of statistics as provided by Elasticsearch</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.count",
            "description": "<p>number of metric values sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.min",
            "description": "<p>minimum value obtained for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.max",
            "description": "<p>maximum value obtained for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.avg",
            "description": "<p>average value across all metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.sum",
            "description": "<p>sum of all sampled metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.sum_of_squares",
            "description": "<p>sum of squares for the given metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.variance",
            "description": "<p>variance of the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation",
            "description": "<p>standard deviation computed for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "statistics.std_deviation_bounds",
            "description": "<p>deviation bounds of the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation_bounds.upper",
            "description": "<p>deviation upper bounds</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation_bounds.lower",
            "description": "<p>deviation lower bounds</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "min",
            "description": "<p>minimum point of the metrics data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.local_timestamp",
            "description": "<p>local time, when the the minimum metric value is sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.server_timestamp",
            "description": "<p>server time, when the the minimum metric value is received by the server</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.host",
            "description": "<p>hostname of the target platform</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.TaskID",
            "description": "<p>identifier of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.type",
            "description": "<p>type of plug-in the metric is associated with</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.metric",
            "description": "<p>metric value associated with a given metric name</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "max",
            "description": "<p>maximum point of the metrics data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.local_timestamp",
            "description": "<p>local time, when the the maximum metric value is sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.host",
            "description": "<p>hostname of the target platform</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.TaskID",
            "description": "<p>identifier of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.type",
            "description": "<p>type of plug-in the metric is associated with</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.metric",
            "description": "<p>metric value associated with a given metric name</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n   \"workflow\": {\n      \"href\": \"http://localhost:3033/v1/phantom_mf/workflows/ms2\"\n   },\n   \"metric\": \"CPU0:core1\",\n   \"statistics\": {\n      \"count\": 1272,\n      \"min\": 25,\n      \"max\": 30,\n      \"avg\": 27.19575471698,\n      \"sum\": 34593,\n      \"sum_of_squares\": 941499,\n      \"variance\": 0.56309518,\n      \"std_deviation\": 0.750396685173416,\n      \"std_deviation_bounds\": {\n         \"upper\": 28.69654808732796,\n         \"lower\": 25.6949613466343\n      }\n   },\n   \"min\": {\n      \"@timestamp\": \"2016-05-17T16:25:48.123\",\n      \"host\": \"node01.excess-project.eu\",\n      \"task\": \"t1\",\n      \"type\": \"performance\",\n      \"CPU0:core0\": 26,\n      \"CPU0:core1\": 25,\n      ...\n   },\n   \"max\": {\n      ...\n   }\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "NoResults",
            "description": "<p>response is empty for the metric.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"error\": \"response is empty for the metric.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/statistics.js",
    "groupTitle": "Statistics"
  },
  {
    "type": "get",
    "url": "/statistics/:workflowID/:taskID",
    "title": "2. Get statistics of a metric across all experiments with given workflow ID and task ID",
    "version": "1.0.0",
    "name": "GetStatsTask",
    "group": "Statistics",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "taskID",
            "description": "<p>Identifier of a task</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>Name of a metric, e.g., metric=CPU0:core1</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "host",
            "description": "<p>Hostname of the system, e.g., host=node01</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "from",
            "description": "<p>Start time of the statistics, e.g., from=2016-05-10T17:35:57.610</p>"
          },
          {
            "group": "Parameter",
            "type": "String",
            "optional": true,
            "field": "to",
            "description": "<p>End time of the statistics, e.g., to=2016-05-10T17:35:57.610</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i 'http://localhost:3033/v1/phantom_mf/statistics/ms2/t1?metric=metric=CPU0:core1&from=2016-05-10T17:35:57.610&to=2016-05-10T17:36:57.610'",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "workflow",
            "description": "<p>workflow-related data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "workflow.href",
            "description": "<p>link to the stored workflow information</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "metric",
            "description": "<p>name of the metric for which statistics are captured</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "statistics",
            "description": "<p>extended set of statistics as provided by Elasticsearch</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.count",
            "description": "<p>number of metric values sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.min",
            "description": "<p>minimum value obtained for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.max",
            "description": "<p>maximum value obtained for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.avg",
            "description": "<p>average value across all metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.sum",
            "description": "<p>sum of all sampled metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.sum_of_squares",
            "description": "<p>sum of squares for the given metric values</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.variance",
            "description": "<p>variance of the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation",
            "description": "<p>standard deviation computed for the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "statistics.std_deviation_bounds",
            "description": "<p>deviation bounds of the given metric</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation_bounds.upper",
            "description": "<p>deviation upper bounds</p>"
          },
          {
            "group": "Success 200",
            "type": "Number",
            "optional": false,
            "field": "statistics.std_deviation_bounds.lower",
            "description": "<p>deviation lower bounds</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "min",
            "description": "<p>minimum point of the metrics data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.local_timestamp",
            "description": "<p>local time, when the the minimum metric value is sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.server_timestamp",
            "description": "<p>server time, when the the minimum metric value is received by the server</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.host",
            "description": "<p>hostname of the target platform</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.TaskID",
            "description": "<p>identifier of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.type",
            "description": "<p>type of plug-in the metric is associated with</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "min.metric",
            "description": "<p>metric value associated with a given metric name</p>"
          },
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "max",
            "description": "<p>maximum point of the metrics data</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.local_timestamp",
            "description": "<p>local time, when the the maximum metric value is sampled</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.host",
            "description": "<p>hostname of the target platform</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.TaskID",
            "description": "<p>identifier of the task</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.type",
            "description": "<p>type of plug-in the metric is associated with</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "max.metric",
            "description": "<p>metric value associated with a given metric name</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n   \"workflow\": {\n      \"href\": \"http://localhost:3033/v1/phantom_mf/workflows/ms2\"\n   },\n   \"metric\": \"CPU0:core1\",\n   \"statistics\": {\n      \"count\": 1272,\n      \"min\": 25,\n      \"max\": 30,\n      \"avg\": 27.19575471698,\n      \"sum\": 34593,\n      \"sum_of_squares\": 941499,\n      \"variance\": 0.56309518,\n      \"std_deviation\": 0.750396685173416,\n      \"std_deviation_bounds\": {\n         \"upper\": 28.69654808732796,\n         \"lower\": 25.6949613466343\n      }\n   },\n   \"min\": {\n      \"@timestamp\": \"2016-05-17T16:25:48.123\",\n      \"host\": \"node01.excess-project.eu\",\n      \"task\": \"t1\",\n      \"type\": \"performance\",\n      \"CPU0:core0\": 26,\n      \"CPU0:core1\": 25,\n      ...\n   },\n   \"max\": {\n      ...\n   }\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "NoResults",
            "description": "<p>response is empty for the metric.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"error\": \"response is empty for the metric.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/statistics.js",
    "groupTitle": "Statistics"
  },
  {
    "type": "get",
    "url": "/verify_es_connection",
    "title": "Returns success (200) when the Server has connection or server-error(503) in other case",
    "version": "1.0.0",
    "name": "verify_es_connection",
    "group": "Testing_Functionality",
    "permission": [
      {
        "name": "user"
      }
    ],
    "description": "<p>The purpose is use for verifying the connectivity of the server with the ElasticSearch Database.</p>",
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -s -XGET http://${server}:${port}/verify_es_connection\nor alternatively use service at https when available SSL certificates.",
        "type": "json"
      }
    ],
    "success": {
      "examples": [
        {
          "title": "Success response:",
          "content": "HTTPS 200 OK",
          "type": "json"
        }
      ]
    },
    "error": {
      "examples": [
        {
          "title": "Response (example):",
          "content": "HTTP/1.1 503 Service Unavailable",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/app_sec.js",
    "groupTitle": "Testing_Functionality"
  },
  {
    "type": "get",
    "url": "/workflows/:workflowID",
    "title": "2. Get information about a specific workflow",
    "version": "1.0.0",
    "name": "GetWorkflow",
    "group": "Workflows",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of a workflow</p>"
          }
        ]
      }
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/workflows/ms2",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "body": [
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "application",
            "description": "<p>Identifier of the workflow</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "author",
            "description": "<p>Author name if provided while registering a new workflow</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "optimization",
            "description": "<p>Optimization criterium: time, energy, balanced</p>"
          },
          {
            "group": "body",
            "type": "Array",
            "optional": false,
            "field": "tasks",
            "description": "<p>List of all tasks, of which the workflow is composed</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "tasks.name",
            "description": "<p>Identifier of the task</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "tasks.exec",
            "description": "<p>Executable for the task</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "tasks.cores_nr",
            "description": "<p>Range of CPU cores used for executing the task on</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"application\": \"ms2\",\n  \"author\": \"Random Guy\",\n  \"optimization\": \"Time\",\n  \"tasks\": [\n    {\n      \"name\": \"T1\",\n      \"exec\": \"/home/ubuntu/ms2/t1.sh\",\n      \"cores_nr\": \"1-2\"\n    },\n    {\n      \"name\": \"T2.1\",\n      \"exec\": \"/home/ubuntu/ms2/t21.sh\",\n      \"previous\": \"T1\",\n      \"cores_nr\": \"1-2\"\n     }\n  ]\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "WorkflowNotAvailable",
            "description": "<p>Given ID does not refer to a workflow.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 404 Not Found\n{\n  \"error\": \"Workflow with the ID '\" + workflowID + \"' not found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/workflows.js",
    "groupTitle": "Workflows"
  },
  {
    "type": "get",
    "url": "/workflows",
    "title": "1. Get a list of all available workflows",
    "version": "1.0.0",
    "name": "GetWorkflows",
    "group": "Workflows",
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "Object",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of the workflow</p>"
          },
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "workflowID.href",
            "description": "<p>Resource location of the given workflow</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"ms2\": {\n    \"href\": \"http://localhost:3033/v1/phantom_mf/workflows/ms2\"\n  },\n  \"infrastructure\": {\n    \"href\": \"http://localhost:3033/v1/phantom_mf/workflows/infrastructure\"\n  }\n}",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/workflows",
        "type": "curl"
      }
    ],
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "WorkflowsNotAvailable",
            "description": "<p>No workflows found.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 404 Not Found\n{\n  \"error\": \"No workflows found.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/workflows.js",
    "groupTitle": "Workflows"
  },
  {
    "type": "put",
    "url": "/workflows/:workflowID",
    "title": "3. Register/Update a workflow with a given workflow ID",
    "version": "1.0.0",
    "name": "PutWorkflowID",
    "group": "Workflows",
    "parameter": {
      "fields": {
        "Parameter": [
          {
            "group": "Parameter",
            "type": "String",
            "optional": false,
            "field": "workflowID",
            "description": "<p>Identifier of the workflow</p>"
          }
        ],
        "body": [
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "application",
            "description": "<p>Identifier of the workflow</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "author",
            "description": "<p>Author name if provided while registering a new workflow</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "optimization",
            "description": "<p>Optimization criterium: time, energy, balanced</p>"
          },
          {
            "group": "body",
            "type": "Array",
            "optional": false,
            "field": "tasks",
            "description": "<p>List of all tasks, of which the workflow is composed</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": false,
            "field": "tasks.name",
            "description": "<p>Identifier of the task</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": true,
            "field": "tasks.exec",
            "description": "<p>Executable for the task</p>"
          },
          {
            "group": "body",
            "type": "String",
            "optional": true,
            "field": "tasks.cores_nr",
            "description": "<p>Range of CPU cores used for executing the task on</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Request-Example:",
          "content": "{\n  \"application\": \"ms2\",\n  \"author\": \"Random Guy\",\n  \"optimization\": \"Time\",\n  \"tasks\": [\n    {\n      \"name\": \"T1\",\n      \"exec\": \"/home/ubuntu/ms2/t1.sh\",\n      \"cores_nr\": \"1-2\"\n    },\n    {\n      \"name\": \"T2.1\",\n      \"exec\": \"/home/ubuntu/ms2/t21.sh\",\n      \"previous\": \"T1\",\n      \"cores_nr\": \"1-2\"\n     }\n  ]\n}",
          "type": "json"
        }
      ]
    },
    "examples": [
      {
        "title": "Example usage:",
        "content": "curl -i http://localhost:3033/v1/phantom_mf/workflows/ms2",
        "type": "curl"
      }
    ],
    "success": {
      "fields": {
        "Success 200": [
          {
            "group": "Success 200",
            "type": "String",
            "optional": false,
            "field": "href",
            "description": "<p>Link to the stored workflow resource</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Success-Response:",
          "content": "HTTP/1.1 200 OK\n{\n  \"href\": \"http://localhost:3033/v1/phantom_mf/workflows/ms2\",\n}",
          "type": "json"
        }
      ]
    },
    "error": {
      "fields": {
        "Error 4xx": [
          {
            "group": "Error 4xx",
            "optional": false,
            "field": "StorageError",
            "description": "<p>Given workflow could not be stored.</p>"
          }
        ]
      },
      "examples": [
        {
          "title": "Error-Response:",
          "content": "HTTP/1.1 500 Internal Server Error\n{\n  \"error\": \"Could not create the workflow.\"\n}",
          "type": "json"
        }
      ]
    },
    "filename": "/home/jmontana/apidocs/server_code/routes/v1/workflows.js",
    "groupTitle": "Workflows"
  }
] });
