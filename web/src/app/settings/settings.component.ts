import { Component, OnInit } from '@angular/core';
import { FormControl, FormGroup } from '@angular/forms';
import { HttpClient, HttpErrorResponse } from '@angular/common/http';
import { catchError } from 'rxjs';

@Component({
  selector: 'app-settings',
  templateUrl: './settings.component.html',
  styleUrls: ['./settings.component.css']
})
export class SettingsComponent implements OnInit {

  settingsForm = new FormGroup({
    avgCountValues: new FormControl(''),
    thresholdCm: new FormControl(''),
  });

  constructor(private httpClient: HttpClient) { }

  ngOnInit(): void {
  }

    onSubmit() {

      const headers = { 'content-type': 'application/json'}
      const body = JSON.stringify( this.settingsForm.value );
      console.log(body);
      this.httpClient.post<any>('http://localhost:4200/api/settings', body, { 'headers' : headers })
        .subscribe()
  }
}
