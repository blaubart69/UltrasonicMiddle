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
      this.httpClient.get<any>( 
        new URL('/api/settings', window.location.href).href
        , { observe : "body", responseType: "json" } )
      .subscribe({
        next : (data) => {
          console.log("data: " + data);
          this.settingsForm.setValue({ 
              "avgCountValues" : data.avg_values
            , "thresholdCm"    : data.threshold_cm
          });
        },
        error: (err) => { 
          alert(err); 
        }
      });
      
    }

    onSubmit() {

      const headers = { 'content-type': 'application/json'}
      //const body = JSON.stringify( this.settingsForm.value );
      const body = {
        "avg_values"   : this.settingsForm.get("avgCountValues")?.value,
        "threshold_cm" : this.settingsForm.get("thresholdCm")?.value,
      };
      console.log(body);

      const url = new URL('/api/settings', window.location.href);

      this.httpClient.post<any>(url.href, body, { 'headers' : headers })
        .subscribe();
  }
}
