from pyexpat import model
from django import forms
from .models import User, Klasse, CoolUser, DataCoolBox
from django.contrib.auth.forms import UserCreationForm

class NewUserForm(UserCreationForm):
	email = forms.EmailField(required=True)

	class Meta:
		model = User
		fields = ("username", "first_name", "last_name", "email", "password1", "password2")

	def save(self, commit=True):
		user = super(NewUserForm, self).save(commit=False)
		user.email = self.cleaned_data['email']
		if commit:
			user.save()
		return user

class KlasseForm(forms.ModelForm):
	class Meta:
		model = Klasse
		fields = "__all__"

class CoolUserForm(forms.ModelForm):
	class Meta:
		model = CoolUser
		fields = ("first_name", "last_name", "klasse", "teacher", "parent" )

class UpdateCoolUserForm(forms.ModelForm):
	class Meta:
		model = CoolUser
		fields = ("first_name", "last_name", "klasse", "parent" )