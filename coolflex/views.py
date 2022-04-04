import random
from datetime import datetime, timedelta
from django.shortcuts import get_object_or_404, render
from django.template import loader
from django.http import HttpResponse, HttpResponseRedirect
from .models import CoolUser, Klasse, User, DataCoolBox
from django.views.generic import ListView
from django.views.generic.edit import CreateView, UpdateView, DeleteView
from django.contrib.auth import authenticate, login, logout
from django.shortcuts import redirect
from django.contrib.auth.decorators import login_required
from django.contrib.auth.mixins import LoginRequiredMixin
from django.contrib.auth.forms import AuthenticationForm
from django.urls import reverse_lazy
from django.contrib import messages
from .forms import NewUserForm, KlasseForm, CoolUserForm, UpdateCoolUserForm
from django.db.models import Q
from django.views.decorators.csrf import csrf_exempt


nicknames = [
    "bestie", "girly", "queen", "king", "boo", "bud", "buddy", "bro", "broski", "bff", "soulmate", "stinky", "homegirl", "bruh", "fave"
]
nickname = random.choice(nicknames)

#TESTING FUNCTIONS
#-------------------------------------------------------------------------------------------------------------------------
coolbox_dict = {1:"Berg", 2:"Nardo", 3:"Singsaker", 4:"Trafikklys"}

#https://gist.github.com/rg3915/db907d7455a4949dbe69
#added a datetime generator for testing purposes
def gen_datetime(year=datetime.now().year):
    # generate a datetime in format yyyy-mm-dd hh:mm:ss.000000
    start = datetime(year, datetime.now().month, datetime.now().day, 5, 00, 00)
    end = start + timedelta(hours=6)
	#end = datetime(min_year, datetime.now().month, 1, 00, 00, 00)
    return start + (end - start) * random.random()


#HELP FUNCTIONS
#--------------------------------------------------------------------------------------------------------------------------

def get_timestamp(user):
	box_data_list = []
	timestamp = datetime.now() #får int feil hvis ikke, default blir .now()
	box_data = DataCoolBox.objects.filter(cooluser_id = user.id)

	for data in box_data:
		box_data_list.append(data)
	
	if (len(box_data_list) > 1):
		timestamp = box_data_list[-1].timestamp
	elif (len(box_data_list) == 1):
		timestamp = box_data_list[0].timestamp
	
	#timestamp = gen_datetime()

	return timestamp

def get_coolbox_location(user, box_dict):
	box_data_list = []
	box_id = 0
	box_data = DataCoolBox.objects.filter(cooluser_id = user.id)

	for data in box_data:
		box_data_list.append(data)
	
	if (len(box_data_list) > 1):
		box_id = box_data_list[-1].box_id
	elif (len(box_data_list) == 1):
		box_id = box_data_list[0].box_id

	return box_dict.get(box_id)

def get_timestamp_text(user):
	timestamp = get_timestamp(user)
	minute = int(timestamp.minute)
	if (minute <= 9):
		minute = "0"+str(minute)
	hour = int(timestamp.hour)
	if (hour <= 9):
		hour = "0"+str(hour)
	day = int(timestamp.day)
	if (day <= 9):
		day = "0"+str(day)
	month = int(timestamp.month)
	timestamp_text = f"{day}/{month} {hour}:{minute}"
	return timestamp_text


def check_timestamp(timestamp):
	#valid timeframe of arrival and todays date
	valid_start_hour = 6
	valid_end_hour = 10
	today = int(datetime.now().day)
	this_month = int(datetime.now().month)
	this_year = int(datetime.now().year)
	#date and hour from timestamp
	hour = int(timestamp.hour)
	minutes = int(timestamp.minute)
	day = int(timestamp.day)
	month = int(timestamp.month)
	year = int(timestamp.year)
	#if sentence to check if valid date
	if ((today == day) and (this_month == month) and (this_year == year)):
		#if sentence to check if cooluser arrived in right timeframe
		if (valid_start_hour <= hour) and (hour < valid_end_hour):
			return 1 #valid timeframe
		elif (hour >= valid_end_hour):
			return 2 #late to school
		else:
			return 3 #invalid timeframe, something is up >:/
	else:
		return False
	

def get_status(user):
	timestamp_status = check_timestamp(get_timestamp(user))
	if (timestamp_status == 1):
		print("Valid date and time")
		return True
	elif (timestamp_status == 2):
		#could return something else if neeeded
		print("Invalid time")
		return 2
	elif (timestamp_status == 3):
		return False
	else:
		print("Invalid date")
		return False


def calculate_cooluser(cooluser_list):
	sum = 0
	total = 0
	for user in cooluser_list:
		status = get_status(user)
		if (status == True) or (status == 2):
			sum += 1
		total += 1
	return sum, total


def get_status_text(user):
	status = get_status(user)
	if (status == True):
		return "Møtt opp"
	elif (status == 2):
		return "Sent oppmøte"
	else:
		return "Ikke møtt opp"

def save_data(string_dict):
	pass
	


#VIEWS
#----------------------------------------------------------------------------------------------------------------------------

# Create your views here.
def home(request):
    return render(request, "coolflex/home.html")

def register_request(request):
	if request.method == "POST":
		form = NewUserForm(request.POST)
		if form.is_valid():
			user = form.save()
			login(request, user)
			messages.success(request, "Registration successful." )
			return redirect("frontpage")
		messages.error(request, "Unsuccessful registration. Invalid information.")
	form = NewUserForm()
	return render (request=request, template_name="coolflex/register.html", context={"register_form":form})


def login_request(request):
	if request.method == "POST":
		form = AuthenticationForm(request, data=request.POST)
		if form.is_valid():
			username = form.cleaned_data.get('username')
			password = form.cleaned_data.get('password')
			user = authenticate(username=username, password=password)
			if user is not None:
				login(request, user)
				messages.info(request, f"You are now logged in as {username}.")
				return redirect("frontpage")
			else:
				messages.error(request,"Invalid username or password.")
		else:
			messages.error(request,"Invalid username or password.")
	form = AuthenticationForm()
	return render(request=request, template_name="coolflex/login.html", context={"login_form":form})

def save_events_json(request):
	if request.is_ajax():
		if request.method == "POST":
			save_data(request.body)
	return HttpResponse("OK")

@login_required
def info(request):
    return render(request, "coolflex/info.html")

#Overview of klasser connected to user, acts as frontpage
@login_required
@csrf_exempt
def frontpage(request):
	name = request.user.username
	klasse_list = []
	user_klasser = Klasse.objects.filter(teacher=request.user)
	for instance in user_klasser:
		instance_dict = instance.__dict__
		instance_dict["grade"] = instance.get_grade()
		instance_dict["letter"] = instance.get_letter()
		coolusers = CoolUser.objects.filter(klasse=instance)
		instance_dict["sum"], instance_dict["total"] = calculate_cooluser(coolusers)
		klasse_list.append(instance_dict)
	context = {"klasse_list" : klasse_list, "name" : name }
	return render(request, "coolflex/frontpage.html", context)


#detail view of each klasse
@login_required
def klasse(request, pk):
	klasse = Klasse.objects.get(id=pk) 
	coolusers = CoolUser.objects.filter(klasse=klasse)
	klassenavn = klasse
	for user in coolusers:
		user.status = get_status_text(user)
		user.location = get_coolbox_location(user, coolbox_dict)
		user.timestamp = get_timestamp_text(user)
	context = {'klasse':klasse, 'coolusers':coolusers, "klassenavn":klassenavn} #context er en ryddig måte å bruke data i template
	return render(request, "coolflex/klasse.html", context)

#detail view of each cooluser, added for easier set-up with URLs with CUD
@login_required
def coolUser(request, pk):
	cooluser = CoolUser.objects.get(id=pk)
	cooluser.status = get_status_text(cooluser)
	cooluser.location = get_coolbox_location(cooluser, coolbox_dict)
	cooluser.timestamp = get_timestamp_text(cooluser)
	context = {'user':cooluser}
	return render(request, "coolflex/cooluser.html", context)

@login_required
@csrf_exempt
def search(request): #https://stackoverflow.com/questions/15050571/django-csrf-token-in-search-result-url
	query = request.GET.get('q', '')
	result = []
	result = CoolUser.objects.filter(
		Q(first_name__icontains=query) | Q(last_name__icontains=query)
	)
	context = {'query':query, 'result':result}
	return render(request, 'coolflex/search_results.html', context)

#CUD for Klasse, using class-based views and forms
class CreateKlasse(CreateView):
	model = Klasse
	template_name="coolflex/klasse_create.html"
	form_class = KlasseForm
	success_url = reverse_lazy('frontpage')

class UpdateKlasse(UpdateView):
	model = Klasse
	template_name="coolflex/klasse_update.html"
	form_class = KlasseForm
	success_url = reverse_lazy('frontpage')

class DeleteKlasse(DeleteView):
	model = Klasse
	template_name = "coolflex/klasse_delete.html"
	success_url = reverse_lazy('frontpage')

#CUD for CoolUser, using class-based views and forms
class CreateCoolUser(CreateView):
	model = CoolUser
	template_name = "coolflex/cooluser_create.html"
	form_class = CoolUserForm
	success_url = reverse_lazy('frontpage')

class UpdateCoolUser(UpdateView):
	model = CoolUser
	template_name = "coolflex/cooluser_update.html"
	form_class = UpdateCoolUserForm
	success_url = reverse_lazy('frontpage')

class DeleteCoolUser(DeleteView):
	model = CoolUser
	template_name="coolflex/cooluser_delete.html"
	success_url = reverse_lazy('frontpage')

