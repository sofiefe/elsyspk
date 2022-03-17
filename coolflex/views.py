import random
#from unicodedata import name
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
from .forms import NewUserForm


klasse_list = []
for instance in Klasse.objects.all():
	instance_dict = instance.__dict__
	instance_dict["grade"] = instance.get_grade()
	instance_dict["letter"] = instance.get_letter()
	klasse_list.append(instance_dict)

"""
cool_user_list = [] 
for instance in CoolUser.objects.all():
	instance_dict = instance.__dict__
	#instance_dict["klasse"] = Klasse.objects.get(instance.klasse.id).navn
	instance_dict["klasse"] = instance.klasse
	cool_user_list.append(instance_dict)
"""


# Create your views here.
def home(request):
    nickname = random.choice(nicknames)
    context = {"name" : nickname }
    return render(request, "coolflex/home.html", context)

nicknames = [
    "bestie", "girly", "queen", "king", "boo", "bud", "buddy", "bro", "broski", "bff", "soulmate", "stinky", "homegirl", "bruh", "fave"
]

@login_required
def frontpage(request):
	context = {"klasse_list" : klasse_list}
	return render(request, "coolflex/frontpage.html", context)

@login_required
def klasse(request, klassenavn):
	klasse = Klasse.objects.get(navn=klassenavn.upper()) #dette kan bli problematisk hvis folk oppretter klasser med navn som ikke har stor bokstav, bør ha begrensinger i models.py
	coolusers = CoolUser.objects.filter(klasse=klasse)
	context = {'klasse':klasse, 'coolusers':coolusers, "klassenavn":klassenavn}
	return render(request, "coolflex/klasse.html", context)

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



class CoolUserView(ListView):
	model = CoolUser
	template_name = 'klasse.html'
