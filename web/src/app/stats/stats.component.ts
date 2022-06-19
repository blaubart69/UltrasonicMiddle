import { Component, OnInit } from '@angular/core';
import { Observable, map, scan } from 'rxjs';
import { retry, shareReplay } from 'rxjs/operators'
import { webSocket } from 'rxjs/webSocket';

/*
typedef struct {
	unsigned long avail0;     
	unsigned long data_ok;    
	unsigned long data_error;
	unsigned long less4;
	unsigned long ne;
	unsigned long notFF;
  unsigned long overwrite;
  unsigned long loops;
} STATS;*/

@Component({
  selector: 'app-stats',
  templateUrl: './stats.component.html',
  styleUrls: ['./stats.component.css']
})
export class StatsComponent implements OnInit {

  public stats$ : Observable<Array<any>> = new Observable<Array<any>>();

  constructor( ) { 
  }

  ngOnInit(): void {

		var url = new URL('/ws/stats', window.location.href);
		url.protocol = url.protocol.replace('http', 'ws')

    this.stats$ = 
			webSocket(url.href)
				.pipe(
					retry(), 
					map( (valJson : any) => valJson ),
          scan( 
            (values,value) => [value].concat(values.slice(0,19))
            , new Array<any>() 
          )
				);
  }
}
