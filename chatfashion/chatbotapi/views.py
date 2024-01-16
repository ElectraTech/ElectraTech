from rest_framework.decorators import api_view
from rest_framework.response import Response
import json
import os
import logging
logging.basicConfig(level=logging.DEBUG, filename='myapp.log', filemode='w')
logger = logging.getLogger()


from openai import OpenAI

client = OpenAI(api_key="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")

example_json = {
"times_stop" : [
{"day_of_week" : "Monday",
"time_stop" : ["10:00","17:00","20:00"]},
]
}
def generate_suggestions(prompt):
    

    completion = client.chat.completions.create(
  model="gpt-3.5-turbo-1106",
  response_format={ "type": "json_object" },
  messages=[
    {"role": "system", "content": "You are a helpful assistant, Provide valid JSON output. The data schema should be like this example:"+ json.dumps(example_json)},
    {"role": "user", "content": prompt}
  ]
)
    finish_reason = completion.choices[0].finish_reason
    if finish_reason == "stop":
        
        data = completion.choices[0].message.content
        print(data)
        times_stop = json.loads(data)
        for i in times_stop['times_stop']:
            print(i['day_of_week'] + " : " + str(i['time_stop']))
        return times_stop
    else:
        print("Error provide more tokens")
        return {"Error":"provide more tokens"}
 

@api_view(['GET', 'POST'])
def chatbot(request):
    if request.method == 'GET':
        return Response({"message": "Hello"})

    if request.method == 'POST':
        time = request.data.get('time', '')
       
        if time:
            
            
            prompt = """
           I give you my electricity usage habits at times during a day from Monday to Sunday. 
           """+time+ """.Track on my electricity usage habits, reccommend what times to turn off electricity.
             Provide one column 'Day_of_week' and one column 'Time' in the JSON format."""
            
            
            json_data = generate_suggestions(prompt)
            return Response(json_data)


       
