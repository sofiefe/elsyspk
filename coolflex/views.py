import random
from datetime import datetime, timedelta
from django.shortcuts import get_object_or_404, render
from django.template import loader
from django.http import HttpResponse
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
from .forms import NewUserForm, KlasseForm, CoolUserForm

nicknames = [
    "bestie", "girly", "queen", "king", "boo", "bud", "buddy", "bro", "broski", "bff", "soulmate", "stinky", "homegirl", "bruh", "fave"
]
nickname = random.choice(nicknames)



#HELP FUNCTIONS
#--------------------------------------------------------------------------------------------------------------------------


def check_timestamp(timestamp):
	#valid timeframe of arrival and todays date
	valid_start_hour = 6
	valid_end_hour = 10
	today = datetime.now().date
	#date and hour from timestamp
	hour = timestamp.hour()
	date = timestamp.date()
	#if sentence to check if valid date
	if (today == date):
		#if sentence to check if cooluser arrived in right timeframe
		if (valid_start_hour<=hour<=valid_end_hour):
			return 1 #valid timeframe
		else:
			return 2 #invalid timeframe, something is up >:/
	else:
		return False
	

def get_status(timestamp):
	timestamp_status = check_timestamp(timestamp)
	if (timestamp_status == 1):
		return True
	elif (timestamp_status == 2):
		#could return something else if neeeded
		return False
	else:
		return False


def calculate_cooluser(cooluser_list):
	sum = 0
	total = 0
	for user in cooluser_list:
		id = user.id
		box_data = DataCoolBox.objects.latest(cooluser_id = id) #søk opp latest
		timestamp = box_data.timestamp
		status = get_status(timestamp)
		if (status == True):
			sum += 1
		total += 1
	return sum, total


#TESTING FUNCTIONSgit 
#-------------------------------------------------------------------------------------------------------------------------
coolbox_dict = {1:"Berg", 2:"Nardo", 3:"Singsaker", 4:"Trafikklys"}

#https://gist.github.com/rg3915/db907d7455a4949dbe69
#added a datetime generator for testing purposes
def gen_datetime(min_year=2022, max_year=datetime.now().year):
    # generate a datetime in format yyyy-mm-dd hh:mm:ss.000000
    start = datetime(min_year, datetime.now().month, 1, 00, 00, 00)
    years = max_year - min_year + 1
    end = start + timedelta(days=30 * years)
	#end = datetime(min_year, datetime.now().month, 1, 00, 00, 00)
    return start + (end - start) * random.random()



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


#Overview of klasser connected to user, acts as frontpage
@login_required
def frontpage(request):
	name = request.user.username
	klasse_list = []
	user_klasser = Klasse.objects.filter(teacher=request.user)
	for instance in user_klasser:
		instance_dict = instance.__dict__
		instance_dict["grade"] = instance.get_grade()
		instance_dict["letter"] = instance.get_letter()
		klasse_list.append(instance_dict)
	context = {"klasse_list" : klasse_list, "name" : name }
	return render(request, "coolflex/frontpage.html", context)


#detail view of each klasse
@login_required
def klasse(request, pk):
	klasse = Klasse.objects.get(id=pk) 
	coolusers = CoolUser.objects.filter(klasse=klasse)
	klassenavn = klasse
	context = {'klasse':klasse, 'coolusers':coolusers, "klassenavn":klassenavn} #context er en ryddig måte å bruke data i template
	return render(request, "coolflex/klasse.html", context)

#detail view of each cooluser, added for easier set-up with URLs with CUD
@login_required
def coolUser(request, pk):
	cooluser = CoolUser.objects.get(id=pk)
	context = {'cooluser':cooluser}
	return render(request, "coolflex/cooluser.html", context)


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
	form_class = CoolUserForm
	success_url = reverse_lazy('frontpage')

class DeleteCoolUser(DeleteView):
	model = CoolUser
	template_name="coolflex/cooluser_delete.html"
	success_url = reverse_lazy('frontpage')