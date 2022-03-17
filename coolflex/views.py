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
from .forms import NewUserForm, KlasseForm



# Create your views here.
def home(request):
    return render(request, "coolflex/home.html")

nicknames = [
    "bestie", "girly", "queen", "king", "boo", "bud", "buddy", "bro", "broski", "bff", "soulmate", "stinky", "homegirl", "bruh", "fave"
]

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

@login_required
def klasse(request, pk):
	klasse = Klasse.objects.get(id=pk) #dette kan bli problematisk hvis folk oppretter klasser med navn som ikke har stor bokstav, bør ha begrensinger i models.py
	coolusers = CoolUser.objects.filter(klasse=klasse)
	klassenavn = klasse
	context = {'klasse':klasse, 'coolusers':coolusers, "klassenavn":klassenavn} #context er en ryddig måte å bruke data i template
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



class CreateKlasse(CreateView):
	model = Klasse
	template_name="coolflex/klasse_create.html"
	#fields = "__all__"
	form_class = KlasseForm

class UpdateKlasse(UpdateView):
	model = Klasse
	template_name="coolflex/klasse_update.html"
	fields = "__all__"


class DeleteKlasse(DeleteView):
	model = Klasse
	template_name="coolflex/klasse_delete.html"
	success_url = 'frontpage'