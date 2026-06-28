// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
	site: 'https://jimakennedy.github.io',
	base: '/poly',
	integrations: [
		starlight({
			title: 'Poly Guide',
			description:
				'Layered Repetition as Sonic Architecture — a guide to polymetric drumming with Poly',
			social: [
				{
					icon: 'github',
					label: 'GitHub',
					href: 'https://github.com/JimAKennedy/poly',
				},
			],
			components: {
				Banner: './src/components/Banner.astro',
			},
			head: [
				{
					tag: 'meta',
					attrs: { property: 'og:image', content: 'https://jimakennedy.github.io/poly/og-image.png' },
				},
				{
					tag: 'meta',
					attrs: { name: 'twitter:image', content: 'https://jimakennedy.github.io/poly/og-image.png' },
				},
			],
			customCss: ['./src/styles/custom.css'],
			sidebar: [
				{ label: 'Introduction', slug: 'introduction' },
				{
					label: 'Chapters',
					items: [
						{ label: '1. Foundations', slug: '01-foundations' },
						{ label: '2. Sub-Saharan Africa', slug: '02-sub-saharan-africa' },
						{ label: '3. Afro-Cuban', slug: '03-afro-cuban' },
						{ label: '4. Afrobeat', slug: '04-afrobeat' },
						{ label: '5. Gamelan', slug: '05-gamelan' },
						{ label: '6. Indian Classical', slug: '06-indian-classical' },
						{ label: '7. Balkan', slug: '07-balkan' },
						{ label: '8. Minimalism', slug: '08-minimalism' },
						{ label: '9. Electronic', slug: '09-electronic' },
						{ label: '10. Brazilian', slug: '10-brazilian' },
						{ label: '11. Funk & Soul', slug: '11-funk-soul' },
						{ label: '12. Jazz', slug: '12-jazz' },
						{ label: '13. Drum & Bass', slug: '13-drum-and-bass' },
						{ label: '14. Synthesis', slug: '14-synthesis' },
						{
							label: '15. Compositional Grammar',
							slug: '15-compositional-grammar',
						},
					],
				},
				{
					label: 'Appendices',
					items: [
						{ label: 'Preset Reference', slug: 'appendix-presets' },
						{
							label: 'Euclidean Reference',
							slug: 'appendix-euclidean-reference',
						},
						{
							label: 'References',
							slug: 'appendix-references',
						},
					],
				},
			],
		}),
	],
});
